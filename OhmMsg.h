#ifndef HEADER_OHMMSG
#define HEADER_OHMMSG

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>

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
	TBool TxTimestamped() const;
	TBool RxTimestamped() const;
	TUint TxTimestamp() const;
	TUint RxTimestamp() const;
	void SetTxTimestamp(TUint aValue);
	void SetRxTimestamp(TUint aValue);
	virtual void Process(IOhmMsgProcessor& aProcessor) = 0;
	virtual void Externalise(IWriter& aWriter) = 0;

protected:
	OhmMsg(OhmMsgFactory& aFactory, TUint aMsgType);
	void Create();
	
private:
	OhmMsgFactory* iFactory;
	TUint iMsgType;
	TUint iRefCount;
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

public:
    TBool Halt() const;
    TBool Lossless() const;
    TBool Timestamped() const;
    TUint Samples() const;
    TUint Frame() const;
    TUint NetworkTimestamp() const;
    TUint MediaLatency() const;
    TUint MediaTimestamp() const;
    TUint64 SampleStart() const;
    TUint64 SamplesTotal() const;
    TUint SampleRate() const;
    TUint BitRate() const;
    TInt VolumeOffset() const;
    TUint BitDepth() const;
    TUint Channels() const;
    const Brx& CodecName() const;
	const Brx& Audio() const;

	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgAudio(OhmMsgFactory& aFactory);
	void Create(IReader& aReader, const OhmHeader& aHeader);	

private:
	OhmHeaderAudio iHeader;
	Bws<kMaxSampleBytes> iAudio;
};

class OhmMsgTrack : public OhmMsg
{
	friend class OhmMsgFactory;

public:
	static const TUint kMaxUriBytes = 1 * 1024;
	static const TUint kMaxMetadataBytes = 4 * 1024;

public:
	TUint Sequence() const;
	const Brx& Uri() const;
	const Brx& Metadata() const;
	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgTrack(OhmMsgFactory& aFactory);
	void Create(IReader& aReader, const OhmHeader& aHeader);	

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

public:
	TUint Sequence() const;
	const Brx& Metatext() const;
	virtual void Process(IOhmMsgProcessor& aProcessor);
	virtual void Externalise(IWriter& aWriter);

private:
	OhmMsgMetatext(OhmMsgFactory& aFactory);
	void Create(IReader& aReader, const OhmHeader& aHeader);	

private:
	TUint iSequence;
	Bws<kMaxMetatextBytes> iMetatext;
};

class IOhmMsgFactory
{
public:
	virtual OhmMsg& Create(IReader& aReader, const OhmHeader& aHeader) = 0;
	virtual OhmMsgAudio& CreateAudio(IReader& aReader, const OhmHeader& aHeader) = 0;
	virtual OhmMsgTrack& CreateTrack(IReader& aReader, const OhmHeader& aHeader) = 0;
	virtual OhmMsgMetatext& CreateMetatext(IReader& aReader, const OhmHeader& aHeader) = 0;
	virtual ~IOhmMsgFactory() {}
};

class OhmMsgFactory : public IOhmMsgFactory, public IOhmMsgProcessor
{
	friend class OhmMsg;

public:
	OhmMsgFactory(TUint aAudioCount, TUint aTrackCount, TUint aMetatextCount);
	virtual OhmMsg& Create(IReader& aReader, const OhmHeader& aHeader);
	virtual OhmMsgAudio& CreateAudio(IReader& aReader, const OhmHeader& aHeader);
	virtual OhmMsgTrack& CreateTrack(IReader& aReader, const OhmHeader& aHeader);
	virtual OhmMsgMetatext& CreateMetatext(IReader& aReader, const OhmHeader& aHeader);
	~OhmMsgFactory();

private:
	void Lock();
	void Unlock();
	void Destroy(OhmMsg& aMsg);
	void Process(OhmMsgAudio& aMsg);
	void Process(OhmMsgTrack& aMsg);
	void Process(OhmMsgMetatext& aMsg);

private:
	Fifo<OhmMsgAudio*> iFifoAudio;
	Fifo<OhmMsgTrack*> iFifoTrack;
	Fifo<OhmMsgMetatext*> iFifoMetatext;
	Mutex iMutex;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_OHMMSG

