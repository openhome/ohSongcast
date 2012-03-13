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
	, iPlay("OHRP", 0)
	, iPlaying("OHRL", 0)
	, iStopped("OHRP", 0)
	, iNullStop("OHRN", 0)
	, iTransportState(eStopped)
	, iPlayMode(eNull)
	, iZoneMode(false)
	, iTerminating(false)
{
	iProtocolMulticast = new OhmProtocolMulticast(iInterface, iTtl, *this, *this);
	iProtocolUnicast = new OhmProtocolUnicast(iInterface, iTtl, *this, *this);
    iThread = new ThreadFunctor("OHRT", MakeFunctor(*this, &OhmReceiver::Run), kThreadPriority, kThreadStackBytes);
    iThread->Start();
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

	iPlay.Signal();

	delete iThread;
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

	iUri.Replace(aUri);

	iPlay.Signal();

	iPlaying.Wait();

	iMutex.Signal();
}

void OhmReceiver::PlayZone(const OpenHome::Uri& /*aUri*/)
{
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
	switch (iPlayMode)
	{
	case eMulticast:
		iProtocolMulticast->Stop();
		break;
	case eUnicast:
		iProtocolUnicast->Stop();
		break;
	case eNull:
		iNullStop.Signal();
		break;
	}

	iStopped.Wait();
}

void OhmReceiver::Run()
{
	for (;;) {
		iPlay.Wait();

		if (iTerminating) {
			break;
		}

		OpenHome::Uri uri(iUri);
	    Endpoint endpoint(uri.Port(), uri.Host());

		if (endpoint.Address() == 0 && endpoint.Port() == 0)
		{
			iPlayMode = eNull;
			iTransportState = eWaiting;
			iDriver->SetTransportState(eWaiting);
			iPlaying.Signal();
			iNullStop.Wait();
		}
		else if (uri.Scheme() == Brn("ohz")) {
			PlayZone(uri);
		}
		else if (uri.Scheme() == Brn("ohm")) {
			iPlayMode = eMulticast;
			iTransportState = eBuffering;
			iDriver->SetTransportState(eBuffering);
			iPlaying.Signal();
			iProtocolMulticast->Play(endpoint);
		}
		else if (uri.Scheme() == Brn("ohu")) {
			iPlayMode = eUnicast;
			iTransportState = eBuffering;
			iDriver->SetTransportState(eBuffering);
			iPlaying.Signal();
			iProtocolUnicast->Play(endpoint);
		}
		else {
			iPlayMode = eNull;
			iTransportState = eWaiting;
			iDriver->SetTransportState(eWaiting);
			iPlaying.Signal();
			iNullStop.Wait();
		}

		iStopped.Signal();
	}
}

// IOhmReceiver

const Brx& OhmReceiver::Add(IOhmAudio& /*aAudio*/)
{
	return (Brx::Empty());
}

void OhmReceiver::SetTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata)
{
	iTrackSequence = aSequence;
	iTrackUri.Replace(aUri);
	iTrackMetadata.Replace(aMetadata);
	iDriver->SetTrack(iTrackSequence, iTrackUri, iTrackMetadata);
}

void OhmReceiver::SetMetatext(const Brx& aValue)
{
	iTrackMetatext.Replace(aValue);
	iDriver->SetMetatext(iTrackMetatext);
}

const Brx& OhmReceiver::Uri() const
{
	return (iTrackUri);
}

const Brx& OhmReceiver::Metadata() const
{
	return (iTrackMetadata);
}

const Brx& OhmReceiver::Metatext() const
{
	return (iTrackMetatext);
}

// IOhmAudioFactory

IOhmAudio& OhmReceiver::Create(OhmHeaderAudio& /*aHeader*/, IReader& /*aReader*/)
{
	return (*(IOhmAudio*)0);
}

