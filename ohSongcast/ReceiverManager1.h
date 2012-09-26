#ifndef HEADER_RECEIVER_MANAGER1
#define HEADER_RECEIVER_MANAGER1

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Timer.h>
#include <OpenHome/Exception.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Av/CpTopology.h>

namespace OpenHome {
namespace Av {

class ReceiverManager1Receiver;

class IReceiverManager1Handler
{
public:
	virtual void ReceiverAdded(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverChanged(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverRemoved(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeControlChanged(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeChanged(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverMuteChanged(ReceiverManager1Receiver& aReceiver) = 0;
	virtual void ReceiverVolumeLimitChanged(ReceiverManager1Receiver& aReceiver) = 0;
	~IReceiverManager1Handler() {}
};

class ReceiverManager1Room;

class ReceiverManager1Receiver  : private INonCopyable
{
	friend class ReceiverManager1Room;

	static const TUint kMaxNameBytes = 20;
	static const TUint kMaxGroupBytes = 20;

public:
	ReceiverManager1Receiver(ReceiverManager1Room& aRoom, const Brx& aGroup, const Brx& aName, TUint aSourceIndex, Net::CpDevice& aDevice);
	const Brx& Room() const;
	const Brx& Group() const;
	const Brx& Name() const;
	TUint SourceIndex() const;
	Net::CpDevice& Device() const;

	TBool HasVolumeControl() const;
	TUint Volume() const;
	TBool Mute() const;
	TUint VolumeLimit() const;
	void SetVolume(TUint aValue);
	void VolumeInc();
	void VolumeDec();
	void SetMute(TBool aValue);

	void SetSourceIndex(TUint aValue);

	void Select();
	TBool Selected() const;

    void AddRef();
    void RemoveRef();
    void SetUserData(void* aValue);
	void* UserData() const;

	void Standby();

	~ReceiverManager1Receiver();

private:
	ReceiverManager1Room& iRoom;
	Bws<kMaxGroupBytes> iGroup;
	Bws<kMaxNameBytes> iName;
	TUint iSourceIndex;
	Net::CpDevice& iDevice;
	void* iUserData;
    TUint iRefCount;
};

class ReceiverManager1;

class ReceiverManager1Room  : private INonCopyable
{
public:
	ReceiverManager1Room(IReceiverManager1Handler& aHandler, IRoom& aRoom);
	const Brx& Name() const;	
	IRoom& Room() const;
    void AddRef();
    void RemoveRef();
	void SourceChanged();
	void Changed();
	void Removed();
	void VolumeControlChanged();
	void VolumeChanged();
	void MuteChanged();
	void VolumeLimitChanged();
	TBool HasVolumeControl() const;
	TUint Volume() const;
	TBool Mute() const;
	TUint VolumeLimit() const;
	void SetVolume(TUint aValue);
	void VolumeInc();
	void VolumeDec();
	void SetMute(TBool aValue);
	void Standby();
	void Select(const ReceiverManager1Receiver& aReceiver);
	TBool Selected(const ReceiverManager1Receiver& aReceiver);
	~ReceiverManager1Room();
private:
	IReceiverManager1Handler& iHandler;
	IRoom& iRoom;
	ReceiverManager1Receiver* iSelected;
	std::vector<ReceiverManager1Receiver*> iReceiverList;
	TUint iRefCount;
};

class ReceiverManager1 : public IHouseHandler, public IReceiverManager1Handler
{
public:
	ReceiverManager1(IReceiverManager1Handler& aHandler);
    void Refresh();
    ~ReceiverManager1();

private:
	// IHouseHandler
    virtual void RoomAdded(IRoom& aRoom);
    virtual void RoomChanged(IRoom& aRoom);
    virtual void RoomRemoved(IRoom& aRoom);
    virtual void RoomStandbyChanged(IRoom& aRoom);
    virtual void RoomSourceChanged(IRoom& aRoom);
    virtual void RoomVolumeControlChanged(IRoom& aRoom);
	virtual void RoomVolumeChanged(IRoom& aRoom);
	virtual void RoomMuteChanged(IRoom& aRoom);
	virtual void RoomVolumeLimitChanged(IRoom& aRoom);

    // IReceiverManager1Handler
	virtual void ReceiverAdded(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverRemoved(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeControlChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverMuteChanged(ReceiverManager1Receiver& aReceiver);
	virtual void ReceiverVolumeLimitChanged(ReceiverManager1Receiver& aReceiver);

private:
	IReceiverManager1Handler& iHandler;
    House* iHouse;
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

