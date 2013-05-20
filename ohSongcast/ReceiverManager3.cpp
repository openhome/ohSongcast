#include "ReceiverManager3.h"

#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Private/Uri.h>

// Assumes only one Receiver per group (UPnP device)

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::Av;

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

// ReceiverManager3Receiver

ReceiverManager3Receiver::ReceiverManager3Receiver(IReceiverManager3Handler& aHandler, ReceiverManager2Receiver& aReceiver, ReceiverManager3& aManager)
	: iHandler(aHandler)
	, iReceiver(aReceiver)
	, iManager(aManager)
	, iRefCount(1)
	, iUserData(0)
{
	iReceiver.AddRef();
	iStatus = EvaluateStatus();
	iHandler.ReceiverAdded(*this);
}

TBool ReceiverManager3Receiver::IsAttachedTo(ReceiverManager2Receiver& aReceiver)
{
	return (&iReceiver == &aReceiver);
}

ReceiverManager3Receiver::EStatus ReceiverManager3Receiver::EvaluateStatus()
{
	return (iManager.Status(iReceiver));
}

const Brx& ReceiverManager3Receiver::Udn() const
{
	return (iReceiver.Device().Udn());
}

const Brx& ReceiverManager3Receiver::Room() const
{
	return (iReceiver.Room());
}

const Brx& ReceiverManager3Receiver::Group() const
{
	return (iReceiver.Group());
}

const Brx& ReceiverManager3Receiver::Name() const
{
	return (iReceiver.Name());
}

ReceiverManager3Receiver::EStatus ReceiverManager3Receiver::Status() const
{
	return (iStatus);
}

TIpAddress ReceiverManager3Receiver::IpAddress() const
{
    CpDevice& dev = iReceiver.Device();

    Brh location;
    dev.GetAttribute("Upnp.Location", location);

    Uri uri(location);
    Endpoint endpoint(0, uri.Host());
    return endpoint.Address();
}

TBool ReceiverManager3Receiver::HasVolumeControl() const
{
	return iReceiver.HasVolumeControl();
}

TUint ReceiverManager3Receiver::Volume() const
{
	return iReceiver.Volume();
}

TBool ReceiverManager3Receiver::Mute() const
{
	return iReceiver.Mute();
}

TUint ReceiverManager3Receiver::VolumeLimit() const
{
	return iReceiver.VolumeLimit();
}

void ReceiverManager3Receiver::SetVolume(TUint aValue)
{
	iManager.SetVolume(iReceiver, aValue);
}

void ReceiverManager3Receiver::VolumeInc()
{
	iManager.VolumeInc(iReceiver);
}

void ReceiverManager3Receiver::VolumeDec()
{
	iManager.VolumeDec(iReceiver);
}

void ReceiverManager3Receiver::SetMute(TBool aValue)
{
	iManager.SetMute(iReceiver, aValue);
}

void ReceiverManager3Receiver::Play()
{
	iManager.Play(iReceiver);
}

void ReceiverManager3Receiver::Stop()
{
	iManager.Stop(iReceiver);
}

void ReceiverManager3Receiver::Standby()
{
	iManager.Standby(iReceiver);
}

void ReceiverManager3Receiver::SetUserData(void* aValue)
{
	iUserData = aValue;
}

void* ReceiverManager3Receiver::UserData() const
{
	return (iUserData);
}

void ReceiverManager3Receiver::AddRef()
{
    iRefCount++;
}

void ReceiverManager3Receiver::RemoveRef()
{
	if (--iRefCount == 0) {
		delete (this);
    }
}

void ReceiverManager3Receiver::Changed()
{
	EStatus status = EvaluateStatus();

	if (iStatus != status) {
		iStatus = status;
		iHandler.ReceiverChanged(*this);
	}
}

void ReceiverManager3Receiver::Removed()
{
	iHandler.ReceiverRemoved(*this);
	RemoveRef();
}

void ReceiverManager3Receiver::VolumeControlChanged()
{
	iHandler.ReceiverVolumeControlChanged(*this);
}

void ReceiverManager3Receiver::VolumeChanged()
{
	iHandler.ReceiverVolumeChanged(*this);
}

void ReceiverManager3Receiver::MuteChanged()
{
	iHandler.ReceiverMuteChanged(*this);
}

void ReceiverManager3Receiver::VolumeLimitChanged()
{
	iHandler.ReceiverVolumeLimitChanged(*this);
}

ReceiverManager3Receiver::~ReceiverManager3Receiver()
{
	iReceiver.RemoveRef();
}

// ReceiverManager

ReceiverManager3::ReceiverManager3(Net::CpStack& aCpStack, IReceiverManager3Handler& aHandler, const Brx& aUri, const Brx& aMetadata)
	: iHandler(aHandler)
	, iUri(aUri)
	, iMetadata(aMetadata)
{
	iReceiverManager = new ReceiverManager2(aCpStack, *this);
}

void ReceiverManager3::Refresh()
{
	iReceiverManager->Refresh();
}

void ReceiverManager3::SetMetadata(const Brx& aMetadata)
{
	iMetadata.Replace(aMetadata);
}

ReceiverManager3Receiver::EStatus ReceiverManager3::Status(ReceiverManager2Receiver& aReceiver)
{
    if (!aReceiver.Selected()) {
        return (ReceiverManager3Receiver::eDisconnected);
    }

	Bws<kMaxUriBytes> uri;
	aReceiver.SenderUri(uri);

	if (uri != iUri) {
		return (ReceiverManager3Receiver::eDisconnected);
	}

	Bws<kMaxTransportStateBytes> state;
	aReceiver.TransportState(state);

	if (state == Brn("Stopped") ) {
		return (ReceiverManager3Receiver::eDisconnected);
	}

	if (state == Brn("Buffering") ) {
		return (ReceiverManager3Receiver::eConnecting);
	}

	return (ReceiverManager3Receiver::eConnected);
}

void ReceiverManager3::SetVolume(ReceiverManager2Receiver& aReceiver, TUint aValue)
{
	aReceiver.SetVolume(aValue);
}

void ReceiverManager3::VolumeInc(ReceiverManager2Receiver& aReceiver)
{
	aReceiver.VolumeInc();
}

void ReceiverManager3::VolumeDec(ReceiverManager2Receiver& aReceiver)
{
	aReceiver.VolumeDec();
}

void ReceiverManager3::SetMute(ReceiverManager2Receiver& aReceiver, TBool aValue)
{
	aReceiver.SetMute(aValue);
}

void ReceiverManager3::Play(ReceiverManager2Receiver& aReceiver)
{
	aReceiver.SetSender(iUri, iMetadata);
	aReceiver.Play();
}

void ReceiverManager3::Stop(ReceiverManager2Receiver& aReceiver)
{
	aReceiver.Stop();
}

void ReceiverManager3::Standby(ReceiverManager2Receiver& aReceiver)
{
	aReceiver.Standby();
}

ReceiverManager3::~ReceiverManager3()
{
	delete (iReceiverManager);
}

// IReceiverManager2Handler

void ReceiverManager3::ReceiverAdded(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTrace, "ReceiverManager3::ReceiverAdded ");
    LOG(kTrace, aReceiver.Room());
    LOG(kTrace, ":");
    LOG(kTrace, aReceiver.Group());
    LOG(kTrace, "\n");

	ReceiverManager3Receiver* receiver = new ReceiverManager3Receiver(iHandler, aReceiver, *this);
	aReceiver.SetUserData(receiver);
}

void ReceiverManager3::ReceiverChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTrace, "ReceiverManager3::ReceiverChanged ");
    LOG(kTrace, aReceiver.Room());
    LOG(kTrace, ":");
    LOG(kTrace, aReceiver.Group());
    LOG(kTrace, "\n");

	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->Changed();
}

void ReceiverManager3::ReceiverRemoved(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTrace, "ReceiverManager3::ReceiverRemoved ");
    LOG(kTrace, aReceiver.Room());
    LOG(kTrace, ":");
    LOG(kTrace, aReceiver.Group());
    LOG(kTrace, "\n");

	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->Removed();
}

void ReceiverManager3::ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager3::ReceiverVolumeControlChanged\n");
	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
    LOG(kTopology, "ReceiverManager3::~ReceiverVolumeControlChanged %x\n", receiver);
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->VolumeControlChanged();
}

void ReceiverManager3::ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager3::ReceiverVolumeChanged\n");
	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
    LOG(kTopology, "ReceiverManager3::~ReceiverVolumeChanged %x\n", receiver);
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->VolumeChanged();
}

void ReceiverManager3::ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager3::ReceiverMuteChanged\n");
	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
    LOG(kTopology, "ReceiverManager3::~ReceiverMuteChanged %x\n", receiver);
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->MuteChanged();
}

void ReceiverManager3::ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager3::ReceiverVolumeLimitChanged\n");
	ReceiverManager3Receiver* receiver = (ReceiverManager3Receiver*)(aReceiver.UserData());
    LOG(kTopology, "ReceiverManager3::~ReceiverVolumeLimitChanged %x\n", receiver);
	ASSERT(receiver);
	ASSERT(receiver->IsAttachedTo(aReceiver));
	receiver->VolumeLimitChanged();
}
