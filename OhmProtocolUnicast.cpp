#include "OhmReceiver.h"
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Debug.h>

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

using namespace OpenHome;
using namespace OpenHome::Net;

// OhmProtocolUnicast

OhmProtocolUnicast::OhmProtocolUnicast(TIpAddress aInterface, TUint aTtl, IOhmReceiver& aReceiver, IOhmAudioFactory& aAudioFactory)
    : iInterface(aInterface)
	, iTtl(aTtl)
	, iReceiver(&aReceiver)
	, iAudioFactory(&aAudioFactory)
    , iReadBuffer(iSocket)
    , iTimerJoin(MakeFunctor(*this, &OhmProtocolUnicast::SendJoin))
    , iTimerListen(MakeFunctor(*this, &OhmProtocolUnicast::SendListen))
    , iTimerLeave(MakeFunctor(*this, &OhmProtocolUnicast::TimerLeaveExpired))
{
}

void OhmProtocolUnicast::SetInterface(TIpAddress aValue)
{
	iInterface = aValue;
}

void OhmProtocolUnicast::SetTtl(TUint aValue)
{
	iTtl = aValue;
}

/*
	virtual const Brx& Add(OhmAudio& aAudio) = 0; // returns array of missed frame numbers
	virtual void SetTrack(const Brx& aUri, const Brx& aMetadata)= 0;
	virtual void SetMetatext(const Brx& aValue) = 0;
*/

void OhmProtocolUnicast::HandleAudio(const OhmHeader& aHeader)
{
	OhmHeaderAudio headerAudio;
	
	headerAudio.Internalise(iReadBuffer, aHeader);

	IOhmAudio& audio = iAudioFactory->Create(headerAudio, iReadBuffer);

	if (iSlaveCount > 0)
	{
		WriterBuffer writer(iMessageBuffer);

		writer.Flush();

		aHeader.Externalise(writer);
		headerAudio.Externalise(writer);
        writer.Write(audio.Samples());

        for (TUint i = 0; i < iSlaveCount; i++) {
        	iSocket.Send(iMessageBuffer, iSlaveList[i]);
        }
	}
                
	const Brx& frames = iReceiver->Add(audio);

	TUint bytes = frames.Bytes();

	if (bytes > 0)
	{
		Bws<OhmHeader::kHeaderBytes + 400> buffer;

		WriterBuffer writer(buffer);

		OhmHeaderResend headerResend(bytes / 4);

		OhmHeader header(OhmHeader::kMsgTypeResend, headerResend.MsgBytes());

		header.Externalise(writer);
		headerResend.Externalise(writer);
		writer.Write(frames);
		
		try {
			iSocket.Send(buffer, iEndpoint);
		}
		catch (NetworkError&)
		{
			LOG(kMedia, "OhmProtocolUnicast::HandleAudio NetworkError\n");
		}
	}
}

void OhmProtocolUnicast::HandleTrack(const OhmHeader& aHeader)
{
	OhmHeaderTrack headerTrack;
	headerTrack.Internalise(iReadBuffer, aHeader);

	TUint sequence = headerTrack.Sequence();
	const Brx& uri = iReadBuffer.Read(headerTrack.UriBytes());
	const Brx& metadata = iReadBuffer.Read(headerTrack.MetadataBytes());
	iReceiver->SetTrack(sequence, uri, metadata);

	if (iSlaveCount > 0)
	{
		WriterBuffer writer(iMessageBuffer);

		writer.Flush();

		aHeader.Externalise(writer);
		headerTrack.Externalise(writer);
        writer.Write(iReceiver->Uri());
        writer.Write(iReceiver->Metadata());

        for (TUint i = 0; i < iSlaveCount; i++) {
        	iSocket.Send(iMessageBuffer, iSlaveList[i]);
        }
	}
}

void OhmProtocolUnicast::HandleMetatext(const OhmHeader& aHeader)
{
	OhmHeaderMetatext headerMetatext;
	headerMetatext.Internalise(iReadBuffer, aHeader);

	iReceiver->SetMetatext(iReadBuffer.Read(headerMetatext.MetatextBytes()));

	if (iSlaveCount > 0)
	{
		WriterBuffer writer(iMessageBuffer);

		writer.Flush();

		aHeader.Externalise(writer);
		headerMetatext.Externalise(writer);
        writer.Write(iReceiver->Metatext());

        for (TUint i = 0; i < iSlaveCount; i++) {
        	iSocket.Send(iMessageBuffer, iSlaveList[i]);
        }
	}
}

void OhmProtocolUnicast::HandleSlave(const OhmHeader& aHeader)
{
    OhmHeaderSlave headerSlave;
    headerSlave.Internalise(iReadBuffer, aHeader);
        	
    iSlaveCount = headerSlave.SlaveCount();

	ReaderBinary reader(iReadBuffer);

    for (TUint i = 0; i < iSlaveCount; i++) {
        TIpAddress address = reader.ReadUintBe(4);
        TUint port = reader.ReadUintBe(2);
        iSlaveList[i].SetAddress(address);
        iSlaveList[i].SetPort(port);
    }
}


void OhmProtocolUnicast::Play(const Endpoint& aEndpoint)
{
    LOG(kMedia, ">OhmProtocolUnicast::Play\n");

	iLeaving = false;

	iSlaveCount = 0;

	iEndpoint.Replace(aEndpoint);

	iSocket.OpenUnicast(iInterface, iTtl);

    try {
        OhmHeader header;

        SendJoin();

		// Phase 1, periodically send join until Track and Metatext have been received

		TBool joinComplete = false;
		TBool receivedTrack = false;
		TBool receivedMetatext = false;

		while (!joinComplete) {
            try {
                header.Internalise(iReadBuffer);

				switch(header.MsgType()) {
				case OhmHeader::kMsgTypeJoin:
				case OhmHeader::kMsgTypeListen:
				case OhmHeader::kMsgTypeLeave:
				case OhmHeader::kMsgTypeResend:
					break;
				case OhmHeader::kMsgTypeAudio:
					HandleAudio(header);
					break;
				case OhmHeader::kMsgTypeTrack:
					HandleTrack(header);
					receivedTrack = true;
					joinComplete = receivedMetatext;
					break;
				case OhmHeader::kMsgTypeMetatext:
					HandleMetatext(header);
					receivedMetatext = true;
					joinComplete = receivedTrack;
					break;
				case OhmHeader::kMsgTypeSlave:
					HandleSlave(header);
				}

                iReadBuffer.ReadFlush();
			}
            catch (OhmError&) {
                LOG(kMedia, "-OhmProtocolUnicast::Play header invalid\n");
            }
		}
            
		iTimerJoin.Cancel();

		// Phase 2, periodically send listen if required

	    iTimerListen.FireIn((kTimerListenTimeoutMs >> 2) - Random(kTimerListenTimeoutMs >> 3)); // listen primary timeout
	    
        for (;;) {
            try {
                header.Internalise(iReadBuffer);

				switch(header.MsgType()) {
				case OhmHeader::kMsgTypeJoin:
				case OhmHeader::kMsgTypeLeave:
				case OhmHeader::kMsgTypeSlave:
				case OhmHeader::kMsgTypeResend:
					break;
				case OhmHeader::kMsgTypeListen:
                    iTimerListen.FireIn((kTimerListenTimeoutMs >> 1) - Random(kTimerListenTimeoutMs >> 3)); // listen secondary timeout
					break;
				case OhmHeader::kMsgTypeAudio:
					HandleAudio(header);
					break;
				case OhmHeader::kMsgTypeTrack:
					HandleTrack(header);
					break;
				case OhmHeader::kMsgTypeMetatext:
					HandleMetatext(header);
					break;
				}

                iReadBuffer.ReadFlush();
			}
            catch (OhmError&) {
                LOG(kMedia, "-OhmProtocolUnicast::Play header invalid\n");
            }
		}
    }
    catch (ReaderError&) {
        LOG(kMedia, "-OhmProtocolUnicast::Play Reader Error (Interrupted!)\n");
    }
    
    iReadBuffer.ReadFlush();

	iLeaving = false;    

   	iTimerJoin.Cancel();
    iTimerListen.Cancel();
	iTimerLeave.Cancel();
    
	iSocket.Close();

    LOG(kMedia, "<OhmProtocolUnicast::Play\n");
}

void OhmProtocolUnicast::Stop()
{
    iLeaving = true;
    iTimerLeave.FireIn(kTimerLeaveTimeoutMs);
}

void OhmProtocolUnicast::EmergencyStop()
{
	SendLeave();
    iReadBuffer.ReadInterrupt();
}

void OhmProtocolUnicast::SendJoin()
{
    LOG(kMedia, "OhmProtocolUnicast::SendJoin\n");
    Send(OhmHeader::kMsgTypeJoin);
    iTimerJoin.FireIn(kTimerJoinTimeoutMs);
}

void OhmProtocolUnicast::SendListen()
{
    LOG(kMedia, "OhmProtocolUnicast::SendListen\n");
    Send(OhmHeader::kMsgTypeListen);
    iTimerListen.FireIn((kTimerListenTimeoutMs >> 2) - Random(kTimerListenTimeoutMs >> 3)); // listen primary timeout
}

void OhmProtocolUnicast::SendLeave()
{
    LOG(kMedia, "OhmProtocolUnicast::SendLeave\n");
    Send(OhmHeader::kMsgTypeLeave);
}

void OhmProtocolUnicast::Send(TUint aType)
{
    Bws<OhmHeader::kHeaderBytes> buffer;
    WriterBuffer writer(buffer);

    OhmHeader msg(aType, 0);
    msg.Externalise(writer);
    
    try {
        iSocket.Send(buffer, iEndpoint);
    }
    catch (NetworkError&)
    {
        LOG(kMedia, "OhmProtocolUnicast::Send NetworkError\n");
    }
}

void OhmProtocolUnicast::TimerLeaveExpired()
{
    LOG(kMedia, "OhmProtocolUnicast::TimerLeaveExpired\n");

	if (iLeaving) {
		SendLeave();
		iReadBuffer.ReadInterrupt();
	}
}

