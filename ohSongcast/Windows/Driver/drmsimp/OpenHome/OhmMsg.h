#ifndef HEADER_OHMMSG
#define HEADER_OHMMSG

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Fifo.h>

#include "Ohm.h"

#include <wdm.h>

// Ohm Header

//Offset    Bytes                   Desc
//0         4                       Ascii representation of "Ohm "
//4         1                       Major Version
//5         1                       Msg Type
//6         2                       Total Bytes (Absolutely all bytes in the entire frame)

// Audio Header

//Offset    Bytes                   Desc
//0         1                       Msg Header Bytes (without the codec name)
//1         1                       Flags (lsb first: halt flag, lossless flag, timestamped flag, resent flag all other bits 0)
//2         2                       Samples in this msg
//4         4                       Frame
//8         4                       Network Timestamp
//12        4                       Media Latency (delay through audio buffers)
//16        4                       Media Timestamp
//20        8                       Sample Start (first sample's offset from the beginiing of this track)
//28        8                       Samples Total (total samples for this track)
//36        4                       Sample Rate
//40        4                       Bit Rate
//44        2                       Volume Offset
//46        1                       Bit depth of audio (16, 24)
//47        1                       Channels
//48        1                       Reserved (must be zero)
//49        1                       Codec Name Bytes
//50        n                       Codec Name
//50 + n    Msg Total Bytes - Msg Header Bytes - Code Name Bytes (Sample data in big endian, channels interleaved, packed)

typedef struct
{
	TUint8 iMagic[4]; // "Ohm "
	TUint8 iMajorVersion; // 1
	TUint8 iMsgType; // 3 - Audio
	TUint16 iTotalBytes;
	TUint8 iAudioHeaderBytes;
	TUint8 iAudioFlags;
	TUint16 iAudioSamples;
	TUint32 iAudioFrame;
	TUint32 iAudioNetworkTimestamp;
	TUint32 iAudioMediaLatency;
	TUint32 iAudioMediaTimestamp;
	TUint32 iAudioSampleStartHi;
	TUint32 iAudioSampleStartLo;
	TUint32 iAudioSamplesTotalHi;
	TUint32 iAudioSamplesTotalLo;
	TUint32 iAudioSampleRate;
	TUint32 iAudioBitRate;
	TUint16 iAudioVolumeOffset;
	TUint8 iAudioBitDepth;
	TUint8 iAudioChannels;
	TUint8 iReserved;
	TUint8 iCodecNameBytes;  // 6
	TUint8 iCodecName[6]; // "PCM   "
} OHMHEADER, *POHMHEADER;

namespace OpenHome {
namespace Net {

class OhmMsgAudio;
class OhmMsgTrack;
class OhmMsgMetatext;
class OhmMsgFactory;

class IOhmMsgProcessor
{
public:
	virtual void Process(OhmMsgAudio& aMsg) = 0;
	virtual void Process(OhmMsgTrack& aMsg) = 0;
	virtual void Process(OhmMsgMetatext& aMsg) = 0;
	virtual ~IOhmMsgProcessor() {}
};

class OhmMsg
{
public:
	void AddRef();
	void RemoveRef();
	TUint ResendCount() const;
	void IncrementResendCount();
	TBool TxTimestamped() const;
	TBool RxTimestamped() const;
	TUint TxTimestamp() const;
	TUint RxTimestamp() const;
	void SetTxTimestamp(TUint aValue);
	void SetRxTimestamp(TUint aValue);
	virtual void Process(IOhmMsgProcessor& aProcessor) = 0;
	virtual void Externalise(IWriter& aWriter) = 0;
	void* operator new(size_t aBytes);
	void operator delete(void* aPtr);

protected:
	OhmMsg(OhmMsgFactory& aFactory, TUint aMsgType);
	void Create();

private:
	virtual void Destroy();
	
private:
	OhmMsgFactory* iFactory;
	TUint iMsgType;
	TUint iRefCount;
	TUint iResendCount;
	TBool iTxTimestamped;
	TBool iRxTimestamped;
	TUint iTxTimestamp;
	TUint iRxTimestamp;
};

class OhmMsgAudio : public OhmMsg
{
	friend class OhmMsgFactory;

public:
	static const TUint kMaxSampleBytes = 8 * 1024;
	static const TUint kMaxCodecBytes = 256;

private:
    static const TUint kHeaderBytes = 50; // not including codec name
    static const TUint kReserved = 0;
    static const TUint kFlagHalt = 1;
    static const TUint kFlagLossless = 2;
    static const TUint kFlagTimestamped = 4;
    static const TUint kFlagResent = 8;

public:
	PMDL Mdl() const;
	TUint Bytes() const;
	TUint Frame() const;
	TBool Resent() const;
	void SetResent(TBool aValue);

	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgAudio(OhmMsgFactory& aFactory);
	void Create(PMDL aMdl, TUint aBytes);
	virtual void Destroy();

private:
	PMDL iMdl;
	TUint iBytes;
};

class OhmMsgTrack : public OhmMsg
{
	friend class OhmMsgFactory;

public:
	static const TUint kMaxUriBytes = 1 * 1024;
	static const TUint kMaxMetadataBytes = 4 * 1024;

private:
    static const TUint kHeaderBytes = 12;

public:
	TUint Sequence() const;
	const Brx& Uri() const;
	const Brx& Metadata() const;
	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgTrack(OhmMsgFactory& aFactory);
	void Create(TUint aSequence, const Brx& aUri, const Brx& aMetadata);

private:
	TUint iSequence;
	Bws<kMaxUriBytes> iUri;
	Bws<kMaxMetadataBytes> iMetadata;
};

class OhmMsgMetatext : public OhmMsg
{
	friend class OhmMsgFactory;

public:
	static const TUint kMaxMetatextBytes = 1 * 1024;

private:
    static const TUint kHeaderBytes = 8;

public:
	TUint Sequence() const;
	const Brx& Metatext() const;
	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgMetatext(OhmMsgFactory& aFactory);
	void Create(TUint aSequence, const Brx& aMetatext);

private:
	TUint iSequence;
	Bws<kMaxMetatextBytes> iMetatext;
};

class OhmMsgFactory : public IOhmMsgProcessor
{
	friend class OhmMsg;

	static const TUint kMaxAudioMessages = 300;
	static const TUint kMaxTrackMessages = 10;
	static const TUint kMaxMetatextMessages = 10;

public:
	OhmMsgFactory();
	virtual OhmMsgAudio& CreateAudio(PMDL aMdl, TUint aBytes);
	virtual OhmMsgTrack& CreateTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata);
	virtual OhmMsgMetatext& CreateMetatext(TUint aSequence, const Brx& aMetatext);
	void* operator new(size_t aBytes);
	void operator delete(void* aPtr);
	~OhmMsgFactory();

private:
	void Destroy(OhmMsg& aMsg);
	void Process(OhmMsgAudio& aMsg);
	void Process(OhmMsgTrack& aMsg);
	void Process(OhmMsgMetatext& aMsg);

private:
	KSPIN_LOCK iSpinLock;
	FifoLite<OhmMsgAudio*, kMaxAudioMessages> iFifoAudio;
	FifoLite<OhmMsgTrack*, kMaxTrackMessages> iFifoTrack;
	FifoLite<OhmMsgMetatext*, kMaxMetatextMessages> iFifoMetatext;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_OHMMSG

