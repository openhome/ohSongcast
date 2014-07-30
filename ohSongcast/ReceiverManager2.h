#ifndef HEADER_RECEIVER_MANAGER2
#define HEADER_RECEIVER_MANAGER2

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Timer.h>
#include <OpenHome/Exception.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/CpAvOpenhomeOrgReceiver1.h>

#include "ReceiverManager1.h"

namespace OpenHome {
    namespace Net {
        class CpStack;
    } // namespace Net
namespace Av {

class ReceiverManager2Receiver;

class IReceiverManager2Handler
{
public:
	virtual void ReceiverAdded(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverChanged(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverRemoved(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver) = 0;
	~IReceiverManager2Handler() {}
};

typedef void (IReceiverManager2Handler::*IReceiverManager2HandlerFunction)(ReceiverManager2Receiver&);

class ReceiverManager2Job
{
public:
	ReceiverManager2Job(IReceiverManager2Handler& aHandler);
    virtual ~ReceiverManager2Job() {}
	void Set(ReceiverManager2Receiver& aReceiver, IReceiverManager2HandlerFunction aFunction);
    virtual void Execute();

private:
	IReceiverManager2Handler* iHandler;
	ReceiverManager2Receiver* iReceiver;
	IReceiverManager2HandlerFunction iFunction;
};


class ReceiverManager2Receiver
{
public:
	ReceiverManager2Receiver(IReceiverManager2Handler& aHandler, ReceiverManager1Receiver& aReceiver);
	Net::CpDevice& Device() const;
	const Brx& Room() const;
	const Brx& Group() const;
	const Brx& Name() const;
	TBool Selected() const;
	void TransportState(Bwx& aValue) const;
	void SenderUri(Bwx& aValue) const;
	void SenderMetadata(Bwx& aValue) const;
	TBool HasVolumeControl() const;
	TUint Volume() const;
	TBool Mute() const;
	TUint VolumeLimit() const;

	void SetVolume(TUint aValue);
	void VolumeInc();
	void VolumeDec();
	void SetMute(TBool aValue);
	void SetSender(const Brx& aUri, const Brx& aMetadata);
	void Play();
	void Stop();
	void Standby();

	void AddRef();
    void RemoveRef();
    void SetUserData(void* aValue);
	void* UserData() const;

	void ChangedSelected(); 
	void Removed();

	void VolumeControlChanged();
	void VolumeChanged();
	void MuteChanged();
	void VolumeLimitChanged();

	void CallbackPlay(Net::IAsync& aAsync);		
	void CallbackStop(Net::IAsync& aAsync);		
	void CallbackSetSender(Net::IAsync& aAsync);		

	void EventReceiverInitialEvent();
	void EventReceiverTransportStateChanged();	
	void EventReceiverUriChanged();	

	~ReceiverManager2Receiver();

private:
	IReceiverManager2Handler& iHandler;
	ReceiverManager1Receiver& iReceiver;
	TBool iActive;
	mutable Mutex iMutex;
	Net::CpProxyAvOpenhomeOrgReceiver1* iServiceReceiver;
    TUint iRefCount;
	void* iUserData;
	Net::FunctorAsync iFunctorStop;
	Net::FunctorAsync iFunctorPlay;
	Net::FunctorAsync iFunctorSetSender;
	Brhz iTransportState;
	Brhz iUri;
	Brhz iMetadata;
	TBool iHasVolumeControl;
	TUint iVolume;
	TBool iMute;
	TUint iVolumeLimit;
};

class ReceiverManager2 : public IReceiverManager1Handler, public IReceiverManager2Handler
{
	static const TUint kMaxJobCount = 20;

public:
	ReceiverManager2(Net::CpStack& aCpStack, IReceiverManager2Handler& aHandler);
    void Refresh();
    virtual ~ReceiverManager2();

private:
	// IReceiverManager1Handler
	virtual void ReceiverAdded(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverRemoved(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeControlChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverMuteChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeLimitChanged(ReceiverManager1Receiver& aReceiver);

	// IReceiverManager2Handler
	virtual void ReceiverAdded(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverChanged(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverRemoved(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver);
	virtual void ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver);

	void Run();

private:
	IReceiverManager2Handler& iHandler;
	Fifo<ReceiverManager2Job*> iFree;
	Fifo<ReceiverManager2Job*> iReady;
    ReceiverManager1* iReceiverManager;
	ThreadFunctor* iThread;
};


/*
	enum EStatus {
		eOff,
		eOn,
		eListening
	};

	EStatus Status() const;
	TUint ReceiverCount() const;
	const Brx& ReceiverRoom(TUint aIndex) const;
	const Brx& ReceiverGroup(TUint aIndex) const;
	void Select(Brx& aReceiverRoom, Brx& aReceiverGroup);
*/

} // namespace Av
} // namespace OpenHome

#endif // HEADER_RECEIVER_MANAGER1

