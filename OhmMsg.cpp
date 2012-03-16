#include "OhmMsg.h"

using namespace OpenHome;
using namespace OpenHome::Net;

// OhmMsg

OhmMsg::OhmMsg(OhmMsgFactory& aFactory, TUint aMsgType)
	: iFactory(&aFactory)
	, iMsgType(aMsgType)
	, iRefCount(0)
{
}

void OhmMsg::AddRef()
{
	iFactory->Lock();

	iRefCount++;

	iFactory->Unlock();
}

void OhmMsg::RemoveRef()
{
	iFactory->Lock();

	if (--iRefCount == 0) {
		iFactory->Destroy(*this);
	}

	iFactory->Unlock();
}

void OhmMsg::Create()
{
	iRefCount = 1;
}
	
// OhmMsgAudio

OhmMsgAudio::OhmMsgAudio(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeAudio)
{
}

void OhmMsgAudio::Create(IReader& aReader, const OhmHeader& aHeader)
{
	iHeader.Internalise(aReader, aHeader);
	//iAudio.Replace(aReader.Read(iHeader.AudioBytes()));
	OhmMsg::Create();
}

TBool OhmMsgAudio::Halt() const
{
	return (iHeader.Halt());
}

TBool OhmMsgAudio::Lossless() const
{
	return (iHeader.Lossless());
}

TBool OhmMsgAudio::Timestamped() const
{
	return (iHeader.Timestamped());
}

TUint OhmMsgAudio::Samples() const
{
	return (iHeader.Samples());
}

TUint OhmMsgAudio::Frame() const
{
	return (iHeader.Frame());
}

TUint OhmMsgAudio::NetworkTimestamp() const
{
	return (iHeader.NetworkTimestamp());
}

TUint OhmMsgAudio::MediaLatency() const
{
	return (iHeader.MediaLatency());
}

TUint OhmMsgAudio::MediaTimestamp() const
{
	return (iHeader.MediaTimestamp());
}

TUint64 OhmMsgAudio::SampleStart() const
{
	return (iHeader.SampleStart());
}

TUint64 OhmMsgAudio::SamplesTotal() const
{
	return (iHeader.SamplesTotal());
}

TUint OhmMsgAudio::SampleRate() const
{
	return (iHeader.SampleRate());
}

TUint OhmMsgAudio::BitRate() const
{
	return (iHeader.BitRate());
}

TInt OhmMsgAudio::VolumeOffset() const
{
	return (iHeader.VolumeOffset());
}

TUint OhmMsgAudio::BitDepth() const
{
	return (iHeader.Halt());
}

TUint OhmMsgAudio::Channels() const
{
	return (iHeader.Channels());
}

const Brx& OhmMsgAudio::CodecName() const
{
	return (iHeader.CodecName());
}

const Brx& OhmMsgAudio::Audio() const
{
	return (iAudio);
}

void OhmMsgAudio::Process(IOhmMsgProcessor& aProcessor)
{
	aProcessor.Process(*this);
}

void OhmMsgAudio::Externalise(IWriter& aWriter)
{
	aWriter.Write(Brn("HELLO"));
}


// OhmMsgTrack

OhmMsgTrack::OhmMsgTrack(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeTrack)
{
}

void OhmMsgTrack::Create(IReader& aReader, const OhmHeader& aHeader)
{
	OhmHeaderTrack header;
	header.Internalise(aReader, aHeader);
	iUri.Replace(aReader.Read(header.UriBytes()));
	iMetadata.Replace(aReader.Read(header.MetadataBytes()));
	iSequence = header.Sequence();
	OhmMsg::Create();
}

TUint OhmMsgTrack::Sequence() const
{
	return (iSequence);
}

const Brx& OhmMsgTrack::Uri() const
{
	return (iUri);
}

const Brx& OhmMsgTrack::Metadata() const
{
	return (iMetadata);
}

void OhmMsgTrack::Process(IOhmMsgProcessor& aProcessor)
{
	aProcessor.Process(*this);
}

void OhmMsgTrack::Externalise(IWriter& aWriter)
{
	aWriter.Write(Brn("HELLO"));
}


// OhmMsgMetatext

OhmMsgMetatext::OhmMsgMetatext(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeMetatext)
{
}

void OhmMsgMetatext::Create(IReader& aReader, const OhmHeader& aHeader)
{
	OhmHeaderMetatext header;
	header.Internalise(aReader, aHeader);
	iMetatext.Replace(aReader.Read(header.MetatextBytes()));
	iSequence = header.Sequence();
	OhmMsg::Create();
}

TUint OhmMsgMetatext::Sequence() const
{
	return (iSequence);
}

const Brx& OhmMsgMetatext::Metatext() const
{
	return (iMetatext);
}

void OhmMsgMetatext::Process(IOhmMsgProcessor& aProcessor)
{
	aProcessor.Process(*this);
}

void OhmMsgMetatext::Externalise(IWriter& aWriter)
{
	aWriter.Write(Brn("HELLO"));
}

// OhmMsgFactory

OhmMsgFactory::OhmMsgFactory(TUint aAudioCount, TUint aTrackCount, TUint aMetatextCount)
	: iFifoAudio(aAudioCount)
	, iFifoTrack(aTrackCount)
	, iFifoMetatext(aMetatextCount)
	, iMutex("OHMF")
{
	for (TUint i = 0; i < aAudioCount; i++) {
		iFifoAudio.Write(new OhmMsgAudio(*this));
	}

	for (TUint i = 0; i < aTrackCount; i++) {
		iFifoTrack.Write(new OhmMsgTrack(*this));
	}

	for (TUint i = 0; i < aMetatextCount; i++) {
		iFifoMetatext.Write(new OhmMsgMetatext(*this));
	}

}

OhmMsg& OhmMsgFactory::Create(IReader& aReader, const OhmHeader& aHeader)
{
	switch (aHeader.MsgType())
	{
	case OhmHeader::kMsgTypeAudio:
		return (CreateAudio(aReader, aHeader));
	case OhmHeader::kMsgTypeTrack:
		return (CreateTrack(aReader, aHeader));
	case OhmHeader::kMsgTypeMetatext:
		return (CreateMetatext(aReader, aHeader));
	default:
		ASSERTS();
	}

	return (*(OhmMsg*)0);
}

OhmMsgAudio& OhmMsgFactory::CreateAudio(IReader& aReader, const OhmHeader& aHeader)
{
	OhmMsgAudio* msg = iFifoAudio.Read();
	msg->Create(aReader, aHeader);
	return (*msg);
}

OhmMsgTrack& OhmMsgFactory::CreateTrack(IReader& aReader, const OhmHeader& aHeader)
{
	OhmMsgTrack* msg = iFifoTrack.Read();
	msg->Create(aReader, aHeader);
	return (*msg);
}

OhmMsgMetatext& OhmMsgFactory::CreateMetatext(IReader& aReader, const OhmHeader& aHeader)
{
	OhmMsgMetatext* msg = iFifoMetatext.Read();
	msg->Create(aReader, aHeader);
	return (*msg);
}

void OhmMsgFactory::Lock()
{
	iMutex.Wait();
}

void OhmMsgFactory::Unlock()
{
	iMutex.Signal();
}

void OhmMsgFactory::Destroy(OhmMsg& aMsg)
{
	aMsg.Process(*this);
}

void OhmMsgFactory::Process(OhmMsgAudio& aMsg)
{
	iFifoAudio.Write(&aMsg);
}

void OhmMsgFactory::Process(OhmMsgTrack& aMsg)
{
	iFifoTrack.Write(&aMsg);
}

void OhmMsgFactory::Process(OhmMsgMetatext& aMsg)
{
	iFifoMetatext.Write(&aMsg);
}

OhmMsgFactory::~OhmMsgFactory()
{
}

