#include "OhmMsg.h"
#include <OpenHome/Arch.h>

#include <ntddk.h>

using namespace OpenHome;
using namespace OpenHome::Net;

// OhmMsg

OhmMsg::OhmMsg(OhmMsgFactory& aFactory, TUint aMsgType)
	: iFactory(&aFactory)
	, iMsgType(aMsgType)
	, iRefCount(0)
	, iResendCount(0)
	, iTxTimestamped(false)
	, iRxTimestamped(false)
	, iTxTimestamp(0)
	, iRxTimestamp(0)
{
}

void* OhmMsg::operator new(size_t aBytes)
{
	return (ExAllocatePoolWithTag(NonPagedPool, aBytes, '2ten'));
}

void  OhmMsg::operator delete(void* aPtr)
{
	ExFreePoolWithTag(aPtr, '2ten');
}

void OhmMsg::AddRef()
{
	iRefCount++;
}

void OhmMsg::RemoveRef()
{
	if (--iRefCount == 0) {
		Destroy();
		iFactory->Destroy(*this);
	}
}

TUint OhmMsg::ResendCount() const
{
	return (iResendCount);
}

void OhmMsg::IncrementResendCount()
{
	iResendCount++;
}

TBool OhmMsg::TxTimestamped() const
{
	return (iTxTimestamped);
}

TBool OhmMsg::RxTimestamped() const
{
	return (iRxTimestamped);
}

TUint OhmMsg::TxTimestamp() const
{
	return (iTxTimestamp);
}

TUint OhmMsg::RxTimestamp() const
{
	return (iRxTimestamp);
}

void OhmMsg::SetTxTimestamp(TUint aValue)
{
	iTxTimestamp = aValue;
	iTxTimestamped = true;
}

void OhmMsg::SetRxTimestamp(TUint aValue)
{
	iRxTimestamp = aValue;
	iRxTimestamped = true;
}

void OhmMsg::Create()
{
	iRefCount = 1;
	iResendCount = 0;
	iTxTimestamp = 0;
	iRxTimestamp = 0;
	iTxTimestamped = false;
	iRxTimestamped = false;
}

void OhmMsg::Destroy()
{
}
	
// OhmMsgAudio

OhmMsgAudio::OhmMsgAudio(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeAudio)
{
}

void OhmMsgAudio::Create(PMDL aMdl, TUint aBytes)
{
	OhmMsg::Create();
	iMdl = aMdl;
	iBytes = aBytes;
}

PMDL OhmMsgAudio::Mdl() const
{
	return (iMdl);
}

TUint OhmMsgAudio::Bytes() const
{
	return (iBytes);
}

TUint OhmMsgAudio::Frame() const
{
	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(iMdl);
	return (Arch::BigEndian4(header->iAudioFrame));
}

TBool OhmMsgAudio::Resent() const
{
	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(iMdl);
	return ((header->iAudioFlags & 0x08) != 0);
}

void OhmMsgAudio::SetResent(TBool aValue)
{
	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(iMdl);

	if (aValue) {
		header->iAudioFlags |= 0x08;
	}
	else {
		header->iAudioFlags &= 0xf7;
	}
}

void OhmMsgAudio::Process(IOhmMsgProcessor& aProcessor)
{
	aProcessor.Process(*this);
}

void OhmMsgAudio::Externalise(IWriter& /*aWriter*/)
{
}

void OhmMsgAudio::Destroy()
{
	PMDL mdl = iMdl;

	while (mdl != NULL)
	{
		ExFreePoolWithTag(MmGetMdlVirtualAddress(mdl), '2ten');
		PMDL next = mdl->Next;
		IoFreeMdl(mdl);
		mdl = next;
	}
}


// OhmMsgTrack

OhmMsgTrack::OhmMsgTrack(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeTrack)
{
}

void OhmMsgTrack::Create(TUint aSequence, const Brx& aUri, const Brx& aMetadata)
{
	OhmMsg::Create();

	iSequence = aSequence;
	iUri.Replace(aUri);
	iMetadata.Replace(aMetadata);
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
	OhmHeader header(OhmHeader::kMsgTypeTrack, kHeaderBytes + iUri.Bytes() + iMetadata.Bytes());

	header.Externalise(aWriter);

    WriterBinary writer(aWriter);
    writer.WriteUint32Be(iSequence);
    writer.WriteUint32Be(iUri.Bytes());
    writer.WriteUint32Be(iMetadata.Bytes());
	writer.Write(iUri);
	writer.Write(iMetadata);

	aWriter.WriteFlush();
}


// OhmMsgMetatext

OhmMsgMetatext::OhmMsgMetatext(OhmMsgFactory& aFactory)
	: OhmMsg(aFactory, OhmHeader::kMsgTypeMetatext)
{
}

void OhmMsgMetatext::Create(TUint aSequence, const Brx& aMetatext)
{
	OhmMsg::Create();

	iSequence = aSequence;
	iMetatext.Replace(aMetatext);
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
	OhmHeaderMetatext headerMetatext(iSequence, iMetatext);
	
	OhmHeader header(OhmHeader::kMsgTypeMetatext, kHeaderBytes + iMetatext.Bytes());

	header.Externalise(aWriter);

	WriterBinary writer(aWriter);
    writer.WriteUint32Be(iSequence);
    writer.WriteUint32Be(iMetatext.Bytes());
	writer.Write(iMetatext);

	aWriter.WriteFlush();
}

// OhmMsgFactory

OhmMsgFactory::OhmMsgFactory()
{
	KeInitializeSpinLock(&iSpinLock);

	for (TUint i = 0; i < kMaxAudioMessages; i++) {
		iFifoAudio.Write(new OhmMsgAudio(*this));
	}

	for (TUint i = 0; i < kMaxTrackMessages; i++) {
		iFifoTrack.Write(new OhmMsgTrack(*this));
	}

	for (TUint i = 0; i < kMaxMetatextMessages; i++) {
		iFifoMetatext.Write(new OhmMsgMetatext(*this));
	}

}

void* OhmMsgFactory::operator new(size_t aBytes)
{
	return (ExAllocatePoolWithTag(NonPagedPool, aBytes, '2ten'));
}

void  OhmMsgFactory::operator delete(void* aPtr)
{
	ExFreePoolWithTag(aPtr, '2ten');
}

OhmMsgAudio& OhmMsgFactory::CreateAudio(PMDL aMdl, TUint aBytes)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	OhmMsgAudio* msg = iFifoAudio.Read();
	KeReleaseSpinLock(&iSpinLock, oldIrql);
	msg->Create(aMdl, aBytes);
	return (*msg);
}

OhmMsgTrack& OhmMsgFactory::CreateTrack(TUint aSequence, const Brx& aUri, const Brx& aMetadata)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	OhmMsgTrack* msg = iFifoTrack.Read();
	KeReleaseSpinLock(&iSpinLock, oldIrql);
	msg->Create(aSequence, aUri, aMetadata);
	return (*msg);
}

OhmMsgMetatext& OhmMsgFactory::CreateMetatext(TUint aSequence, const Brx& aMetatext)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	OhmMsgMetatext* msg = iFifoMetatext.Read();
	KeReleaseSpinLock(&iSpinLock, oldIrql);
	msg->Create(aSequence, aMetatext);
	return (*msg);
}

void OhmMsgFactory::Destroy(OhmMsg& aMsg)
{
	aMsg.Process(*this);
}

void OhmMsgFactory::Process(OhmMsgAudio& aMsg)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	iFifoAudio.Write(&aMsg);
	KeReleaseSpinLock(&iSpinLock, oldIrql);
}

void OhmMsgFactory::Process(OhmMsgTrack& aMsg)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	iFifoTrack.Write(&aMsg);
	KeReleaseSpinLock(&iSpinLock, oldIrql);
}

void OhmMsgFactory::Process(OhmMsgMetatext& aMsg)
{
	KIRQL oldIrql;
	KeAcquireSpinLock(&iSpinLock, &oldIrql);
	iFifoMetatext.Write(&aMsg);
	KeReleaseSpinLock(&iSpinLock, oldIrql);
}

OhmMsgFactory::~OhmMsgFactory()
{
}

