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
	, iMutexMode("OHRM")
	, iMutexTransport("OHRT")
	, iPlaying("OHRP", 0)
	, iZoning("OHRZ", 0)
	, iStopped("OHRS", 0)
	, iNullStop("OHRN", 0)
	, iLatency(0)
	, iTransportState(eStopped)
	, iPlayMode(eNone)
	, iZoneMode(false)
	, iTerminating(false)
	, iEndpointNull(0, Brn("0.0.0.0"))
	, iRxZone(iSocketZone)
    , iTimerZoneQuery(MakeFunctor(*this, &OhmReceiver::SendZoneQuery))
	, iFactory(100, 10, 10)
	, iRepairing(false)
{
	iProtocolMulticast = new OhmProtocolMulticast(iInterface, iTtl, *this, iFactory);
	iProtocolUnicast = new OhmProtocolUnicast(iInterface, iTtl, *this, iFactory);
    iThread = new ThreadFunctor("OHRT", MakeFunctor(*this, &OhmReceiver::Run), kThreadPriority, kThreadStackBytes);
    iThread->Start();
    iThreadZone = new ThreadFunctor("OHRZ", MakeFunctor(*this, &OhmReceiver::RunZone), kThreadZonePriority, kThreadZoneStackBytes);
    iThreadZone->Start();
}

OhmReceiver::~OhmReceiver()
{
	iMutexTransport.Wait();

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
	
	iMutexTransport.Signal();
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

	iMutexTransport.Wait();

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

	iMutexMode.Wait();

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
		iZone.Replace(uri.PathAndQuery().Split(1));
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

	iMutexMode.Signal();
	iMutexTransport.Signal();
}

void OhmReceiver::PlayZoneMode(const Brx& aUri)
{
    LOG(kMedia, ">OhmReceiver::PlayZoneMode\n");

	iMutexTransport.Wait();

	OpenHome::Uri uri;

	try {
		uri.Replace(aUri);
	}
	catch (UriError&) {
	}

	Endpoint endpoint(uri.Port(), uri.Host());

	iMutexMode.Wait();

	if (iEndpoint.Equals(endpoint)) {
		iMutexMode.Signal();
		iMutexTransport.Signal();
	    LOG(kMedia, "<OhmReceiver::PlayZoneMode ignored\n");
		return;
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

	iLatency = 0;

	iRepairing = RepairClear();

	iEndpoint.Replace(endpoint);

	if (iEndpoint.Equals(iEndpointNull))
	{
		iPlayMode = eNull;
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
		iThread->Signal();
		iPlaying.Wait();
	}
	else if (uri.Scheme() == Brn("ohm")) {
		iPlayMode = eMulticast;
		iTransportState = eBuffering;
		iDriver->SetTransportState(eBuffering);
		iThread->Signal();
		iPlaying.Wait();
	}
	else if (uri.Scheme() == Brn("ohu")) {
		iPlayMode = eUnicast;
		iTransportState = eBuffering;
		iDriver->SetTransportState(eBuffering);
		iThread->Signal();
		iPlaying.Wait();
	}
	else {
		iPlayMode = eNull;
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
		iThread->Signal();
		iPlaying.Wait();
	}

	iMutexMode.Signal();
	iMutexTransport.Signal();

    LOG(kMedia, "<OhmReceiver::PlayZoneMode\n");
}

void OhmReceiver::Stop()
{
	iMutexTransport.Wait();

	if (iTransportState != eStopped)
	{
		StopLocked();
		iTransportState = eStopped;
		iDriver->SetTransportState(eStopped);
	}

	iMutexTransport.Signal();
}

void OhmReceiver::StopLocked()
{
	iMutexMode.Wait();
	iMutexTransport.Signal();

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

	iLatency = 0;

	iRepairing = RepairClear();

	iMutexTransport.Wait();
	iMutexMode.Signal();
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

void OhmReceiver::SendZoneQuery()
{
	LOG(kMedia, "OhmReceiver::SendZoneQuery ");
	LOG(kMedia, iZone);
	LOG(kMedia, "\n");
                
	OhzHeaderZoneQuery headerZoneQuery(iZone);
	OhzHeader header(OhzHeader::kMsgTypeZoneQuery, headerZoneQuery.MsgBytes());

	WriterBuffer writer(iTxZone);
        
    writer.Flush();
    header.Externalise(writer);
    headerZoneQuery.Externalise(writer);
    writer.Write(iZone);

	iSocketZone.Send(iTxZone);

	iTimerZoneQuery.FireIn(kTimerZoneQueryDelayMs);
}

void OhmReceiver::RunZone()
{
    for (;;) {
        LOG(kMedia, "OhmReceiver::RunZone wait\n");
        
        iThreadZone->Wait();

		if (iTerminating) {
			break;
		}

		iSocketZone.Open(iInterface, iTtl);
		iZoning.Signal();
		SendZoneQuery();

        LOG(kMedia, "OhmReceiver::RunZone go\n");
        
		try {
			for (;;) {
				OhzHeader header;
	        
				for (;;) {
        			try {
						header.Internalise(iRxZone);
						break;
					}
					catch (OhzError&) {
						LOG(kMedia, "OhmReceiver::RunZone received error\n");
						iRxZone.ReadFlush();
					}
				}

				if (header.MsgType() == OhzHeader::kMsgTypeZoneUri) {
					OhzHeaderZoneUri headerZoneUri;
					headerZoneUri.Internalise(iRxZone, header);

					Brn msgZone = iRxZone.Read(headerZoneUri.ZoneBytes());
					Brn msgUri = iRxZone.Read(headerZoneUri.UriBytes());

			        LOG(kMedia, "OhmReceiver::RunZone received ");
					LOG(kMedia, msgZone);
					LOG(kMedia, " = ");
					LOG(kMedia, msgUri);
					LOG(kMedia, "\n");
                
					if (msgZone == iZone)
					{
						iTimerZoneQuery.Cancel();
						PlayZoneMode(msgUri);
					}
				}

				iRxZone.ReadFlush();
			}
		}
		catch (ReaderError&) {
		}

		iRxZone.ReadFlush();
		iTimerZoneQuery.Cancel();
		iSocketZone.Close();
        iStopped.Signal();
	}
}

TUint OhmReceiver::Latency(OhmMsgAudio& aMsg)
{
	TUint latency = aMsg.MediaLatency();

	if (latency != 0) {
		return (latency);
	}

	return (kDefaultLatency);
}

TBool OhmReceiver::RepairClear()
{
	return (false);
}

TBool OhmReceiver::RepairBegin(OhmMsgAudio& aMsg)
{
	iDriver->Add(aMsg);
	return (false);
}

TBool OhmReceiver::Repair(OhmMsgAudio& aMsg)
{
	iDriver->Add(aMsg);
	return (false);
}

// IOhmReceiver

void OhmReceiver::Add(OhmMsg& aMsg)
{
	iMutexTransport.Wait();

	aMsg.Process(*this);

	iMutexTransport.Signal();
}

// IOhmMsgProcessor

void OhmReceiver::Process(OhmMsgAudio& aMsg)
{
	if (iLatency == 0) {
		iFrame = aMsg.Frame();
		iLatency = Latency(aMsg);
		iTransportState = ePlaying;
		iDriver->SetTransportState(ePlaying);
		iDriver->Add(aMsg);
	}
	else if (iRepairing) {
		iRepairing = Repair(aMsg);
	}
	else {
		TInt diff = aMsg.Frame() - iFrame;

		if (diff == 1) {
			iFrame++;
			iDriver->Add(aMsg);
		}
		else if (diff < 1) {
			aMsg.RemoveRef();
		}
		else {
			iRepairing = RepairBegin(aMsg);
		}
	}
}

void OhmReceiver::Process(OhmMsgTrack& aMsg)
{
	if (iTransportState == eBuffering) {
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
	}

	iDriver->Add(aMsg);
}

void OhmReceiver::Process(OhmMsgMetatext& aMsg)
{
	if (iTransportState == eBuffering) {
		iTransportState = eWaiting;
		iDriver->SetTransportState(eWaiting);
	}

	iDriver->Add(aMsg);
}
