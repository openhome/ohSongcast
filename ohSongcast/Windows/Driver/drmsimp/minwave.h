/*++

Copyright (c) 1997-2000  Microsoft Corporation All Rights Reserved

Module Name:

    minwave.h

Abstract:

    Definition of wavecyclic miniport class.

--*/

#ifndef _MSVAD_MINWAVE_H_
#define _MSVAD_MINWAVE_H_

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhmMsg.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Fifo.h>

#include "basewave.h"
#include "network.h"

//=============================================================================
// Referenced Forward
//=============================================================================
class CMiniportWaveCyclicStream;
typedef CMiniportWaveCyclicStream *PCMiniportWaveCyclicStream;

//=============================================================================
// Classes
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// CMiniportWaveCyclic 
//   

class CMiniportWaveCyclic : public CMiniportWaveCyclicMSVAD,  public IMiniportWaveCyclic, public CUnknown
{
    friend class CMiniportWaveCyclicStream;
    friend class CMiniportTopologySimple;

	static const TUint kMaxPipelineMessages = 16;
	static const TUint kMaxHistoryMessages = 100;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportWaveCyclic);

	void UpdateEndpoint(TUint aAddress, TUint aPort, TUint aAdapter);
	void UpdateTtl(TUint aValue);
	void UpdateActive(TUint aValue);
	void UpdateEnabled(TUint aValue);
	void UpdateLatency(TUint aValue);

	virtual void PipelineStop();
	virtual void PipelineSend(TByte* aBuffer, TUint aBytes);
	virtual void SetFormat(TUint aSampleRate, TUint aBitRate, TUint aBitDepth, TUint aChannels);
	void PipelineResend(const OpenHome::Brx& aFrames);

    ~CMiniportWaveCyclic();

    IMP_IMiniportWaveCyclic;

private:
	void UpdateLatencyLocked();
	void SetFormatLocked(TUint aSampleRate, TUint aBitRate, TUint aBitDepth, TUint aChannels);
	OpenHome::Net::OhmMsgAudio* PipelineQueueRemove(SOCKADDR* aAddress);
	void PipelineOutput();
	static NTSTATUS PipelineOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext);
	NTSTATUS PipelineOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp);
	TBool PipelineQueueAddLocked(PMDL* aMdl, TUint* aBytes);
	void PipelineSendNewLocked();
	void PipelineCopyAudioLocked(TByte* aDestination, TByte* aSource, TUint aBytes, TUint aSampleBytes);
	void PipelineSendAddFragmentLocked(TByte* aBuffer, TUint aBytes);
	TBool PipelineSendLocked(TByte* aBuffer, TUint aBytes);
	TBool PipelineStopLocked();
	TBool ResendLocked(OpenHome::Net::OhmMsgAudio& aMsg);

private:
    TBool iCaptureAllocated;
    TBool iRenderAllocated;
	OpenHome::Net::OhmMsgFactory iFactory;
	OpenHome::FifoLite<OpenHome::Net::OhmMsgAudio*, kMaxPipelineMessages> iPipeline;
	OpenHome::FifoLite<OpenHome::Net::OhmMsgAudio*, kMaxHistoryMessages> iHistory;
	KSPIN_LOCK iPipelineSpinLock;
	TBool iPipelineSending;
	TBool iPipelineStopped;
	PMDL iPipelineMdl;
	PMDL iPipelineMdlLast;
	TUint iPipelinePacketBytes;
	TUint iPipelineSampleBytes;
	TUint iPipelineSampleChannelBytes;
	OpenHome::Net::OhmMsgAudio* iPipelineOutputMsg;
	WSK_BUF iPipelineOutputBuf;
	TUint iPipelineAdapter;
	SOCKADDR iPipelineAddress;
	SOCKADDR iPipelineOutputAddress;
	TUint iPipelineFrame;
	OHMHEADER iPipelineHeader;
	TUint iPipelineBytes;
	TUint64 iPipelinePerformanceCounter;

	TUint iEnabled;
	TUint iActive;
	TUint iTtl;
	TUint iAddr;
	TUint iPort;
	TUint iLatency;

	CWinsock* iWsk;
	CSocketOhm* iSocket;
};

typedef CMiniportWaveCyclic *PCMiniportWaveCyclic;

///////////////////////////////////////////////////////////////////////////////
// CMiniportWaveCyclicStream 
//   

class CMiniportWaveCyclicStream : 
    public IDrmAudioStream,
    public CMiniportWaveCyclicStreamMSVAD,
    public CUnknown
{
protected:
    PCMiniportWaveCyclic        m_pMiniportLocal;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportWaveCyclicStream);
    ~CMiniportWaveCyclicStream();

    IMP_IDrmAudioStream;

    NTSTATUS                    Init
    ( 
        IN  PCMiniportWaveCyclic Miniport,
        IN  ULONG               Channel,
        IN  BOOLEAN             Capture,
        IN  PKSDATAFORMAT       DataFormat
    );

    // Friends
    friend class                CMiniportWaveCyclic;
};
typedef CMiniportWaveCyclicStream *PCMiniportWaveCyclicStream;

#endif

