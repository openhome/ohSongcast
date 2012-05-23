#include "ReceiverManager2.h"

#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Debug.h>

// Assumes only one Receiver per group (UPnP device)

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::Av;

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

// ReceiverManager2Job

ReceiverManager2Job::ReceiverManager2Job(IReceiverManager2Handler& aHandler)
{
	iHandler = &aHandler;
	iReceiver = 0;
}
	
void ReceiverManager2Job::Set(ReceiverManager2Receiver& aReceiver, IReceiverManager2HandlerFunction aFunction)
{
	iReceiver = &aReceiver;
	iFunction = aFunction;
	iReceiver->AddRef();
}

void ReceiverManager2Job::Execute()
{
	if (iReceiver) {
		(iHandler->*iFunction)(*iReceiver);
		iReceiver->RemoveRef();
		iReceiver = 0;
	}
	else {
		THROW(ThreadKill);
	}
}

// ReceiverManager2Receiver

ReceiverManager2Receiver::ReceiverManager2Receiver(IReceiverManager2Handler& aHandler, ReceiverManager1Receiver& aReceiver)
	: iHandler(aHandler)
	, iReceiver(aReceiver)
	, iActive(false)
	, iMutex("RM2R")
	, iRefCount(1)
	, iUserData(0)
	, iHasVolumeControl(false)
	, iVolume(0)
	, iMute(false)
	, iVolumeLimit(0)
{
	iReceiver.AddRef();

    iFunctorStop = MakeFunctorAsync(*this, &ReceiverManager2Receiver::CallbackStop);
    iFunctorPlay = MakeFunctorAsync(*this, &ReceiverManager2Receiver::CallbackPlay);
    iFunctorSetSender = MakeFunctorAsync(*this, &ReceiverManager2Receiver::CallbackSetSender);

	iServiceReceiver = new CpProxyAvOpenhomeOrgReceiver1(iReceiver.Device());

	Functor functorInitial = MakeFunctor(*this, &ReceiverManager2Receiver::EventReceiverInitialEvent);

    iServiceReceiver->SetPropertyInitialEvent(functorInitial); 

	iServiceReceiver->Subscribe();
}

CpDevice& ReceiverManager2Receiver::Device() const
{
	return (iReceiver.Device());
}

const Brx& ReceiverManager2Receiver::Room() const
{
	return (iReceiver.Room());
}

const Brx& ReceiverManager2Receiver::Group() const
{
	return (iReceiver.Group());
}

const Brx& ReceiverManager2Receiver::Name() const
{
	return (iReceiver.Name());
}

TBool ReceiverManager2Receiver::Selected() const
{
	return (iReceiver.Selected());
}

void ReceiverManager2Receiver::TransportState(Bwx& aValue) const
{
	iMutex.Wait();
	try {
		aValue.ReplaceThrow(iTransportState);
	}
	catch (BufferOverflow) {
		aValue.Replace(Brx::Empty());
	}
	iMutex.Signal();
}

void ReceiverManager2Receiver::SenderUri(Bwx& aValue) const
{
	iMutex.Wait();
	try {
		aValue.ReplaceThrow(iUri);
	}
	catch (BufferOverflow) {
		aValue.Replace(Brx::Empty());
	}
	iMutex.Signal();
}

void ReceiverManager2Receiver::SenderMetadata(Bwx& aValue) const
{
	iMutex.Wait();
	try {
		aValue.ReplaceThrow(iMetadata);
	}
	catch (BufferOverflow) {
		aValue.Replace(Brx::Empty());
	}
	iMutex.Signal();
}

TBool ReceiverManager2Receiver::HasVolumeControl() const
{
	iMutex.Wait();
	TBool hasVolumeControl = iHasVolumeControl;
	iMutex.Signal();

	return hasVolumeControl;
}

TUint ReceiverManager2Receiver::Volume() const
{
	iMutex.Wait();
	TUint volume = iVolume;
	iMutex.Signal();

	return volume;
}

TBool ReceiverManager2Receiver::Mute() const
{
	iMutex.Wait();
	TBool mute = iMute;
	iMutex.Signal();

	return mute;
}

TUint ReceiverManager2Receiver::VolumeLimit() const
{
	iMutex.Wait();
	TUint volumeLimit = iVolumeLimit;
	iMutex.Signal();

	return volumeLimit;
}
	

// Actions

void ReceiverManager2Receiver::SetVolume(TUint aValue)
{
	iReceiver.SetVolume(aValue);
}

void ReceiverManager2Receiver::VolumeInc()
{
	iReceiver.VolumeInc();
}

void ReceiverManager2Receiver::VolumeDec()
{
	iReceiver.VolumeDec();
}

void ReceiverManager2Receiver::SetMute(TBool aValue)
{
	iReceiver.SetMute(aValue);
}

void ReceiverManager2Receiver::Play()
{
	iReceiver.Select();
	iServiceReceiver->BeginPlay(iFunctorPlay);
}

void ReceiverManager2Receiver::Stop()
{
	iServiceReceiver->BeginStop(iFunctorStop);
}

void ReceiverManager2Receiver::Standby()
{
	iReceiver.Standby();
}

void ReceiverManager2Receiver::SetSender(const Brx& aUri, const Brx& aMetadata)
{
	iServiceReceiver->BeginSetSender(aUri, aMetadata, iFunctorSetSender);
}

// Action callbacks

void ReceiverManager2Receiver::CallbackStop(IAsync& /* aAsync */)
{
}

void ReceiverManager2Receiver::CallbackPlay(IAsync& /* aAsync */)
{
}

void ReceiverManager2Receiver::CallbackSetSender(IAsync& /* aAsync */)
{
}

// Event handlers

void ReceiverManager2Receiver::EventReceiverInitialEvent()
{
    Functor functorTransportState = MakeFunctor(*this, &ReceiverManager2Receiver::EventReceiverTransportStateChanged);
    Functor functorUri = MakeFunctor(*this, &ReceiverManager2Receiver::EventReceiverUriChanged);

    iServiceReceiver->SetPropertyTransportStateChanged(functorTransportState);    
    iServiceReceiver->SetPropertyUriChanged(functorUri); 
      
	iServiceReceiver->PropertyTransportState(iTransportState);
	iServiceReceiver->PropertyUri(iUri);
	iServiceReceiver->PropertyMetadata(iMetadata);

	iMutex.Wait();
	iActive = true;
	TBool hasVolumeControl = iHasVolumeControl;
	iMutex.Signal();

	iHandler.ReceiverAdded(*this);
	if(hasVolumeControl)
	{
		iHandler.ReceiverVolumeControlChanged(*this);
	}
}

void ReceiverManager2Receiver::EventReceiverTransportStateChanged()
{
	iMutex.Wait();
	iServiceReceiver->PropertyTransportState(iTransportState);
	iMutex.Signal();
	iHandler.ReceiverChanged(*this);
}

void ReceiverManager2Receiver::EventReceiverUriChanged()
{
	iMutex.Wait();
	iServiceReceiver->PropertyUri(iUri);
	iServiceReceiver->PropertyMetadata(iMetadata);
	iMutex.Signal();
	iHandler.ReceiverChanged(*this);
}

void ReceiverManager2Receiver::ChangedSelected()
{
	iMutex.Wait();

	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverChanged(*this);
	}
	else
	{
		iMutex.Signal();
	}
}

void ReceiverManager2Receiver::Removed()
{
    // This is called in the ReceiverManager1 thread (which is actually the CpTopology2 thread)
    // and there are event handlers, above, which can get called in eventing threads. There is potential
    // that this function will call the iHandler.ReceiverRemoved thread before the iHandler.ReceiverChanged
    // calls in the eventing threads.
    // This next line ensures that the eventing stops, thus, no further iHandler.ReceiverChanged calls will
    // be made after this iHandler.ReceiverRemoved call.
    iServiceReceiver->Unsubscribe();

	iMutex.Wait();

	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverRemoved(*this);
	}
	else
	{
		iMutex.Signal();
	}


	RemoveRef();
}

void ReceiverManager2Receiver::VolumeControlChanged()
{
	iMutex.Wait();

	iHasVolumeControl = iReceiver.HasVolumeControl();
	iVolume = iReceiver.Volume();
	iMute = iReceiver.Mute();
	iVolumeLimit = iReceiver.VolumeLimit();
	
	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverVolumeControlChanged(*this);
	}
	else
	{
		iMutex.Signal();
	}
}

void ReceiverManager2Receiver::VolumeChanged()
{
	iMutex.Wait();

	iVolume = iReceiver.Volume();
	
	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverVolumeChanged(*this);
	}
	else
	{
		iMutex.Signal();
	}
}

void ReceiverManager2Receiver::MuteChanged()
{
	iMutex.Wait();

	iMute = iReceiver.Mute();
	
	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverMuteChanged(*this);
	}
	else
	{
		iMutex.Signal();
	}
}

void ReceiverManager2Receiver::VolumeLimitChanged()
{
	iMutex.Wait();

	iVolumeLimit = iReceiver.VolumeLimit();
	
	if (iActive) {
		iMutex.Signal();
		iHandler.ReceiverVolumeLimitChanged(*this);
	}
	else
	{
		iMutex.Signal();
	}
}

// Public API

void ReceiverManager2Receiver::AddRef()
{
	iRefCount++;
}

void ReceiverManager2Receiver::RemoveRef()
{
	if (--iRefCount == 0) {
		delete (this);
	}
}

void ReceiverManager2Receiver::SetUserData(void* aValue)
{
	iUserData = aValue;
}

void* ReceiverManager2Receiver::UserData() const
{
	return (iUserData);
}

ReceiverManager2Receiver::~ReceiverManager2Receiver()
{
	delete (iServiceReceiver);
	iReceiver.RemoveRef();
}

// ReceiverManager

ReceiverManager2::ReceiverManager2(IReceiverManager2Handler& aHandler)
	: iHandler(aHandler)
	, iFree(kMaxJobCount)
	, iReady(kMaxJobCount)
{
	for (TUint i = 0; i < kMaxJobCount; i++) {
		iFree.Write(new ReceiverManager2Job(aHandler));
	}
	
	iReceiverManager = new ReceiverManager1(*this);

	iThread = new ThreadFunctor("RM2T", MakeFunctor(*this, &ReceiverManager2::Run));
	iThread->Start();
}

void ReceiverManager2::Refresh()
{
	iReceiverManager->Refresh();
}

ReceiverManager2::~ReceiverManager2()
{
    LOG(kTopology, "ReceiverManager2::~ReceiverManager2\n");

	delete (iReceiverManager);
    
	iReady.Write(iFree.Read()); // this null job causes the thread to complete

	delete (iThread);
	
    LOG(kTopology, "ReceiverManager2::~ReceiverManager2 deleted thread\n");

	for (TUint i = 0; i < kMaxJobCount; i++) {
		delete (iFree.Read());
	}

    LOG(kTopology, "ReceiverManager2::~ReceiverManager2 deleted jobs\n");
}

// IReceiverManager1Handler

void ReceiverManager2::ReceiverAdded(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = new ReceiverManager2Receiver(*this, aReceiver);
	aReceiver.SetUserData(receiver);
}

void ReceiverManager2::ReceiverChanged(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->ChangedSelected();
}

void ReceiverManager2::ReceiverRemoved(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->Removed();
}

void ReceiverManager2::ReceiverVolumeControlChanged(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->VolumeControlChanged();
}

void ReceiverManager2::ReceiverVolumeChanged(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->VolumeChanged();
}

void ReceiverManager2::ReceiverMuteChanged(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->MuteChanged();
}

void ReceiverManager2::ReceiverVolumeLimitChanged(ReceiverManager1Receiver& aReceiver)
{
	ReceiverManager2Receiver* receiver = (ReceiverManager2Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	receiver->VolumeLimitChanged();
}

// IReceiverManager2Handler

void ReceiverManager2::ReceiverAdded(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager2::ReceiverAdded ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverAdded);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverChanged(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager2::ReceiverChanged ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverChanged);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverRemoved(ReceiverManager2Receiver& aReceiver)
{
    LOG(kTopology, "ReceiverManager2::ReceiverRemoved ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverRemoved);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver)
{
	LOG(kTopology, "ReceiverManager2::ReceiverVolumeControlChanged ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
	LOG(kTopology, ":");
    LOG(kTopology, aReceiver.HasVolumeControl() ? Brn("Yes") : Brn("No"));
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverVolumeControlChanged);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver)
{
	LOG(kTopology, "ReceiverManager2::ReceiverVolumeChanged ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverVolumeChanged);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver)
{
	LOG(kTopology, "ReceiverManager2::ReceiverMuteChanged ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverMuteChanged);
	iReady.Write(job);
}

void ReceiverManager2::ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver)
{
	LOG(kTopology, "ReceiverManager2::ReceiverVolumeLimitChanged ");
    LOG(kTopology, aReceiver.Room());
    LOG(kTopology, ":");
    LOG(kTopology, aReceiver.Group());
    LOG(kTopology, "\n");

	ReceiverManager2Job* job = iFree.Read();
	job->Set(aReceiver, &IReceiverManager2Handler::ReceiverVolumeLimitChanged);
	iReady.Write(job);
}


void ReceiverManager2::Run()
{
    LOG(kTopology, "ReceiverManager2::Run Started\n");

    for (;;)
    {
	    LOG(kTopology, "ReceiverManager2::Run wait for job\n");

    	ReceiverManager2Job* job = iReady.Read();
    	
	    LOG(kTopology, "ReceiverManager2::Run execute job\n");

    	try {
	    	job->Execute();
	    	iFree.Write(job);
	    }
	    catch (ThreadKill)
	    {
	    	iFree.Write(job);
	    	break;
	    }
    }

    LOG(kTopology, "ReceiverManager2::Run Exiting\n");
}
