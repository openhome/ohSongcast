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

OhmProtocolUnicast::OhmProtocolUnicast(TIpAddress aInterface, TUint aTtl, IOhmReceiver& aReceiver, IOhmMsgFactory& aFactory)
    : iInterface(aInterface)
	, iTtl(aTtl)
	, iReceiver(&aReceiver)
	, iFactory(&aFactory)
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

void OhmProtocolUnicast::HandleAudio(const OhmHeader& aHeader)
{
	try
	{
		OhmMsgAudio& msg = iFactory->CreateAudio(iReadBuffer, aHeader);

		Broadcast(msg);

		iReceiver->Add(msg);

		if (iLeaving) {
			LOG(kMedia, "OhmProtocolUnicast::HandleAudio leaving detected\n");
			iTimerLeave.Cancel();
			SendLeave();
			iReadBuffer.ReadInterrupt();
		}
	}
	catch (...)
	{
		LOG(kMedia, "OhmProtocolUnicast::HandleAudio error\n");
	}

}

void OhmProtocolUnicast::HandleTrack(const OhmHeader& aHeader)
{
	OhmMsgTrack& msg = iFactory->CreateTrack(iReadBuffer, aHeader);

	Broadcast(msg);

	iReceiver->Add(msg);
}

void OhmProtocolUnicast::HandleMetatext(const OhmHeader& aHeader)
{
	OhmMsgMetatext& msg = iFactory->CreateMetatext(iReadBuffer, aHeader);

	Broadcast(msg);

	iReceiver->Add(msg);
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

void OhmProtocolUnicast::RequestResend(const Brx& aFrames)
{
	TUint bytes = aFrames.Bytes();

	if (bytes > 0)
	{
		Bws<OhmHeader::kHeaderBytes + 400> buffer;

		WriterBuffer writer(buffer);

		OhmHeaderResend headerResend(bytes / 4);

		OhmHeader header(OhmHeader::kMsgTypeResend, headerResend.MsgBytes());

		header.Externalise(writer);
		headerResend.Externalise(writer);
		writer.Write(aFrames);
		
		try {
			iSocket.Send(buffer, iEndpoint);
		}
		catch (NetworkError&)
		{
			LOG(kMedia, "OhmProtocolUnicast::RequestResend NetworkError\n");
		}
	}
}

void OhmProtocolUnicast::Broadcast(OhmMsg& aMsg)
{
	if (iSlaveCount > 0)
	{
		WriterBuffer writer(iMessageBuffer);

		writer.Flush();

		aMsg.Externalise(writer);

        for (TUint i = 0; i < iSlaveCount; i++) {
        	iSocket.Send(iMessageBuffer, iSlaveList[i]);
        }
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
    LOG(kMedia, "OhmProtocolUnicast::Stop\n");
    iLeaving = true;
    iTimerLeave.FireIn(kTimerLeaveTimeoutMs);
}

void OhmProtocolUnicast::EmergencyStop()
{
	SendLeave();
	TimerLeaveExpired();
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
	SendLeave();
	iReadBuffer.ReadInterrupt();
}

