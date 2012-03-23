#ifndef HEADER_OHMMSG
#define HEADER_OHMMSG

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Fifo.h>

#include "Ohm.h"

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

	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgAudio(OhmMsgFactory& aFactory);
	void Create(PMDL aMdl, TUint aBytes);

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

	static const TUint kMaxAudioMessages = 100;
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
	FifoLite<OhmMsgAudio*, kMaxAudioMessages> iFifoAudio;
	FifoLite<OhmMsgTrack*, kMaxTrackMessages> iFifoTrack;
	FifoLite<OhmMsgMetatext*, kMaxMetatextMessages> iFifoMetatext;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_OHMMSG

