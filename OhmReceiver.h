#ifndef HEADER_OHMRECEIVER
#define HEADER_OHMRECEIVER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Net/Core/DvDevice.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Timer.h>
#include <OpenHome/Private/Uri.h>


#include "Ohm.h"

namespace OpenHome {
namespace Net {

enum EOhmReceiverTransportState
{
	ePlaying,
	eStopped,
	eWaiting,
	eBuffering
};

enum EOhmReceiverPlayMode
{
	eMulticast,
	eUnicast,
	eNull,
};

class IOhmReceiverDriver
{
public:
    static const TUint kMaxUriBytes = 1000;
    static const TUint kMaxMetadataBytes = 5000;
    static const TUint kMaxMetatextBytes = 5000;
    virtual void SetAudioFormat(TUint aSampleRate, TUint aBitRate, TUint aChannels, TUint aBitDepth, TBool aLossless, const Brx& aCodecName) = 0;
	virtual void SetTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata) = 0;
	virtual void SetMetatext(const Brx& aValue) = 0;
	virtual void SetTransportState(EOhmReceiverTransportState aValue) = 0;
    virtual void Play(const TByte* aData, TUint aBytes, TUint64 aSampleStart, TUint64 aSamplesTotal) = 0;
	virtual ~IOhmReceiverDriver() {}
};

class IOhmAudio
{
public:
	virtual void AddRef() = 0;
	virtual void RemoveRef() = 0;
	virtual const OhmHeader& Header() = 0;
	virtual const Brx& Samples() = 0;
	virtual ~IOhmAudio() {}
};

class IOhmAudioFactory
{
public:
	virtual IOhmAudio& Create(OhmHeaderAudio& aHeader, IReader& aReader) = 0;
	virtual ~IOhmAudioFactory() {}
};

class IOhmReceiver
{
public:
	virtual const Brx& Add(IOhmAudio& aAudio) = 0; // returns array of missed frame numbers
	virtual void SetTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata) = 0;
	virtual void SetMetatext(const Brx& aValue) = 0;
	virtual const Brx& Uri() const = 0;
	virtual const Brx& Metadata() const = 0;
	virtual const Brx& Metatext() const = 0;
	virtual ~IOhmReceiver() {}
};

class OhmProtocolMulticast
{
    static const TUint kMaxFrameBytes = 16*1024;
    static const TUint kAddMembershipDelayMs = 100;
    
    static const TUint kTimerJoinTimeoutMs = 300;
    static const TUint kTimerListenTimeoutMs = 10000;
    
public:
	OhmProtocolMulticast(TIpAddress aInterface, TUint aTtl, IOhmReceiver& aReceiver, IOhmAudioFactory& aAudioFactory);
	void SetInterface(TIpAddress aValue);
    void SetTtl(TUint aValue);
    void Play(const Endpoint& aEndpoint);
	void Stop();

private:
	void HandleAudio(const OhmHeader& aHeader);
	void HandleTrack(const OhmHeader& aHeader);
	void HandleMetatext(const OhmHeader& aHeader);
    void SendJoin();
    void SendListen();
    void Send(TUint aType);

private:
	TIpAddress iInterface;
	TUint iTtl;
	IOhmReceiver* iReceiver;
	IOhmAudioFactory* iAudioFactory;
    OhmSocket iSocket;
    Srs<kMaxFrameBytes> iReadBuffer;
    Endpoint iEndpoint;
    Timer iTimerJoin;
    Timer iTimerListen;
};

class OhmProtocolUnicast
{
    static const TUint kMaxFrameBytes = 16*1024;
    static const TUint kAddMembershipDelayMs = 100;
    
    static const TUint kTimerJoinTimeoutMs = 300;
    static const TUint kTimerListenTimeoutMs = 10000;
    static const TUint kTimerLeaveTimeoutMs = 50;
	static const TUint kMaxSlaveCount = 4;
    
public:
	OhmProtocolUnicast(TIpAddress aInterface, TUint aTtl, IOhmReceiver& aReceiver, IOhmAudioFactory& aAudioFactory);
	void SetInterface(TIpAddress aValue);
    void SetTtl(TUint aValue);
    void Play(const Endpoint& aEndpoint);
	void Stop();
	void EmergencyStop();

private:
	void HandleAudio(const OhmHeader& aHeader);
	void HandleTrack(const OhmHeader& aHeader);
	void HandleMetatext(const OhmHeader& aHeader);
	void HandleSlave(const OhmHeader& aHeader);
    void SendJoin();
    void SendListen();
    void SendLeave();
    void Send(TUint aType);
    void TimerLeaveExpired();

private:
	TIpAddress iInterface;
	TUint iTtl;
	IOhmReceiver* iReceiver;
	IOhmAudioFactory* iAudioFactory;
    OhmSocket iSocket;
    Srs<kMaxFrameBytes> iReadBuffer;
    Endpoint iEndpoint;
    Timer iTimerJoin;
    Timer iTimerListen;
    Timer iTimerLeave;
	TBool iLeaving;
	TUint iSlaveCount;
    Endpoint iSlaveList[kMaxSlaveCount];
	Bws<kMaxFrameBytes> iMessageBuffer;
};

class OhmReceiver : public IOhmReceiver, public IOhmAudioFactory
{
    static const TUint kThreadPriority = kPriorityNormal;
    static const TUint kThreadStackBytes = 64 * 1024;
	static const TUint kMaxUriBytes = 100;

public:
    OhmReceiver(TIpAddress aInterface, TUint aTtl, IOhmReceiverDriver& aDriver);

	TIpAddress Interface() const;
	TUint Ttl() const;

	void SetInterface(TIpAddress aValue);
    void SetTtl(TUint aValue);

	void Play(const Brx& aUri);
	void Stop();
    
    ~OhmReceiver();

private:
	void Run();
	void StopLocked();
	void PlayZone(const OpenHome::Uri& aUri);

	// IOhmReceiver
	virtual const Brx& Add(IOhmAudio& aAudio);
	virtual void SetTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata);
	virtual void SetMetatext(const Brx& aValue);
	virtual const Brx& Uri() const;
	virtual const Brx& Metadata() const;
	virtual const Brx& Metatext() const;

	// IOhmAudioFactory
	virtual IOhmAudio& Create(OhmHeaderAudio& aHeader, IReader& aReader);

private:
	TIpAddress iInterface;
	TUint iTtl;
    IOhmReceiverDriver* iDriver;
	ThreadFunctor* iThread;
	Mutex iMutex;
	Semaphore iPlay;
	Semaphore iPlaying;
	Semaphore iStopped;
	Semaphore iNullStop;
	EOhmReceiverTransportState iTransportState;
	EOhmReceiverPlayMode iPlayMode;
	TBool iZoneMode;
	TBool iTerminating;
	OhmProtocolMulticast* iProtocolMulticast;
	OhmProtocolUnicast* iProtocolUnicast;
	Bws<kMaxUriBytes> iUri;
	TUint iTrackSequence;
	Bws<IOhmReceiverDriver::kMaxUriBytes> iTrackUri;
	Bws<IOhmReceiverDriver::kMaxMetadataBytes> iTrackMetadata;
	Bws<IOhmReceiverDriver::kMaxMetatextBytes> iTrackMetatext;
};


} // namespace Net
} // namespace OpenHome

#endif // HEADER_OHMRECEIVER

