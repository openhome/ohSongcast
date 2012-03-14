#include "OhmReceiver.h"
#include <OpenHome/Net/Core/DvAvOpenhomeOrgReceiver1.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Debug.h>

#include <stdio.h>

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

using namespace OpenHome;
using namespace OpenHome::Net;

OhmReceiver::OhmReceiver(TIpAddress aInterface, TUint aTtl, IOhmReceiverDriver& aDriver)
	: iInterface(aInterface)
	, iTtl(aTtl)
	, iDriver(&aDriver)
	, iMutex("OHRM")
	, iPlaying("OHRP", 0)
	, iZoning("OHRZ", 0)
	, iStopped("OHRS", 0)
	, iNullStop("OHRN", 0)
	, iTransportState(eStopped)
	, iPlayMode(eNone)
	, iZoneMode(false)
	, iTerminating(false)
	, iEndpointNull(0, Brn("0.0.0.0"))
	, iRxZone(iSocketZone)
{
	iProtocolMulticast = new OhmProtocolMulticast(iInterface, iTtl, *this, *this);
	iProtocolUnicast = new OhmProtocolUnicast(iInterface, iTtl, *this, *this);
    iThread = new ThreadFunctor("OHRT", MakeFunctor(*this, &OhmReceiver::Run), kThreadPriority, kThreadStackBytes);
    iThread->Start();
    iThreadZone = new ThreadFunctor("OHRZ", MakeFunctor(*this, &OhmReceiver::RunZone), kThreadZonePriority, kThreadZoneStackBytes);
    iThreadZone->Start();
}

OhmReceiver::~OhmReceiver()
{
	iMutex.Wait();

	if (iTransportState != eStopped)
	{
		StopLocked();
		iTransportState = eStopped;
		iDriver->SetTransportState(eStopped);
	}

	iTerminating = true;

	iThread->Signal();
	iThreadZone->Signal();

	delete (iThread);
	delete (iThreadZone);
	delete (iProtocolMulticast);
	delete (iProtocolUnicast);
	
	iMutex.Signal();
}

TUint OhmReceiver::Ttl() const
{
	return (iTtl);
}

TIpAddress OhmReceiver::Interface() const
{
	return (iInterface);
}

void OhmReceiver::SetTtl(TUint aValue)
{
	iTtl = aValue;
}

void OhmReceiver::SetInterface(TIpAddress aValue)
{
	iInterface = aValue;
}

void OhmReceiver::Play(const Brx& aUri)
{
    LOG(kMedia, ">OhmReceiver::Play\n");

	iMutex.Wait();

	if (iTransportState != eStopped)
	{
		StopLocked();
		iTransportState = eStopped;
		iDriver->SetTransportState(eStopped);
	}

	OpenHome::Uri uri;

	try {
		uri.Replace(aUri);
	}
	catch (UriError&) {
	}

	iEndpoint.Replace(Endpoint(uri.Port(), uri.Host()));

	if (iEndpoint.Equals(iEndpointNull))
	{
		iZoneMode = false;
		iPlayMode = eNull;
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
		iThread->Signal();
		iPlaying.Wait();
	}
	else if (uri.Scheme() == Brn("ohz") && iEndpoint.Equals(iSocketZone.This())) {
		iZoneMode = true;
		iPlayMode = eNone;
		iTransportState = eBuffering;
		iDriver->SetTransportState(eBuffering);
		iThreadZone->Signal();
		iZoning.Wait();
	}
	else if (uri.Scheme() == Brn("ohm")) {
		iZoneMode = false;
		iPlayMode = eMulticast;
		iTransportState = eBuffering;
		iDriver->SetTransportState(eBuffering);
		iThread->Signal();
		iPlaying.Wait();
	}
	else if (uri.Scheme() == Brn("ohu")) {
		iZoneMode = false;
		iPlayMode = eUnicast;
		iTransportState = eBuffering;
		iDriver->SetTransportState(eBuffering);
		iThread->Signal();
		iPlaying.Wait();
	}
	else {
		iZoneMode = false;
		iPlayMode = eNull;
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
		iThread->Signal();
		iPlaying.Wait();
	}

	iMutex.Signal();
}

void OhmReceiver::Stop()
{
	iMutex.Wait();

	if (iTransportState != eStopped)
	{
		StopLocked();
		iTransportState = eStopped;
		iDriver->SetTransportState(eStopped);
	}

	iMutex.Signal();
}

void OhmReceiver::StopLocked()
{
	if (iZoneMode)
	{
		iSocketZone.ReadInterrupt();
		iStopped.Wait();
	}

	switch (iPlayMode)
	{
	case eNone:
		break;
	case eMulticast:
		iProtocolMulticast->Stop();
		iStopped.Wait();
		break;
	case eUnicast:
		iProtocolUnicast->Stop();
		iStopped.Wait();
		break;
	case eNull:
		iNullStop.Signal();
		iStopped.Wait();
		break;
	}
}

void OhmReceiver::Run()
{
	for (;;) {
		iThread->Wait();

		if (iTerminating) {
			break;
		}

		Endpoint endpoint(iEndpoint);

		switch (iPlayMode) {
		case eMulticast:
			iPlaying.Signal();
			iProtocolMulticast->Play(endpoint);
			break;
		case eUnicast:
			iPlaying.Signal();
			iProtocolUnicast->Play(endpoint);
			break;
		case eNull:
			iPlaying.Signal();
			iNullStop.Wait();
			break;
		}

		iStopped.Signal();
	}
}

void OhmReceiver::RunZone()
{
    for (;;) {
        LOG(kMedia, "OhmSender::RunZone wait\n");
        
        iThreadZone->Wait();

		if (iTerminating) {
			break;
		}

		iSocketZone.Open(iInterface, iTtl);

		iZoning.Signal();

        LOG(kMedia, "OhmSender::RunZone go\n");
        
		try {
			for (;;) {
				OhzHeader header;
	        
				for (;;) {
        			try {
						header.Internalise(iRxZone);
						break;
					}
					catch (OhzError&) {
						LOG(kMedia, "OhmSender::RunZone received error\n");
						iRxZone.ReadFlush();
					}
				}

				/*
				if (header.MsgType() == OhzHeader::kMsgTypeZoneQuery) {
					OhzHeaderZoneQuery headerZoneQuery;
					headerZoneQuery.Internalise(iRxZone, header);

					Brn zone = iRxZone.Read(headerZoneQuery.ZoneBytes());

			        LOG(kMedia, "OhmSender::RunZone received zone query for ");
					LOG(kMedia, zone);
					LOG(kMedia, "\n");
                
					if (zone == iDevice.Udn())
					{
						iMutexZone.Wait();
						SendZoneUri(1);
						iMutexZone.Signal();
					}
				}
				else if (header.MsgType() == OhzHeader::kMsgTypePresetQuery) {
			        LOG(kMedia, "OhmSender::RunZone received preset query\n");
					OhzHeaderPresetQuery headerPresetQuery;
					headerPresetQuery.Internalise(iRxZone, header);
					TUint preset = headerPresetQuery.Preset();

					if (preset > 0) {
						iMutexZone.Wait();
						if (preset == iPreset) {
							SendPresetInfo(1);
						}
						iMutexZone.Signal();
					}
				}

				else {
					LOG(kMedia, "OhmSender::RunZone received message type %d\n", header.MsgType());
				}
				*/

				iRxZone.ReadFlush();
			}
		}
		catch (ReaderError&) {
		}

        iStopped.Signal();
	}
}

// IOhmReceiver

const Brx& OhmReceiver::Add(IOhmAudio& /*aAudio*/)
{
	iMutex.Wait();

	if (iTransportState == eBuffering || iTransportState == eWaiting) {
		iTransportState = ePlaying;
		iDriver->SetTransportState(ePlaying);
	}

	iMutex.Signal();

	printf(".");

	return (Brx::Empty());
}

void OhmReceiver::SetTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata)
{
	iTrackSequence = aSequence;

	iTrackUri.Replace(aUri);
	
	iTrackMetadata.Replace(aMetadata);
	
	iMutex.Wait();

	if (iTransportState == eBuffering) {
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
	}

	iMutex.Signal();

	iDriver->SetTrack(iTrackSequence, iTrackUri, iTrackMetadata);
}

void OhmReceiver::SetMetatext(TUint aSequence, const Brx& aValue)
{
	iMetatextSequence = aSequence;

	iMetatext.Replace(aValue);

	iMutex.Wait();

	if (iTransportState == eBuffering) {
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
	}

	iMutex.Signal();

	iDriver->SetMetatext(iMetatextSequence, iMetatext);
}

TUint OhmReceiver::TrackSequence() const
{
	return (iTrackSequence);
}

const Brx& OhmReceiver::TrackUri() const
{
	return (iTrackUri);
}

const Brx& OhmReceiver::TrackMetadata() const
{
	return (iTrackMetadata);
}

TUint OhmReceiver::MetatextSequence() const
{
	return (iMetatextSequence);
}

const Brx& OhmReceiver::Metatext() const
{
	return (iMetatext);
}

// IOhmAudioFactory

IOhmAudio& OhmReceiver::Create(OhmHeaderAudio& /*aHeader*/, IReader& /*aReader*/)
{
	return (*(IOhmAudio*)0);
}

