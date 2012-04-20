/*
++

Copyright (c) 1997-2000  Microsoft Corporation All Rights Reserved

Module Name:

    minwave.cpp

Abstract:

    Implementation of wavecyclic miniport.

--
*/

#pragma warning (disable : 4127)

#include <msvad.h>
#include <common.h>
#include <ntddk.h>
#include "drmsimp.h"
#include "minwave.h"
#include "wavtable.h"
#include "network.h"

using namespace OpenHome;
using namespace OpenHome::Net;

#pragma code_seg("PAGE")

//=============================================================================
// CMiniportWaveCyclic
//=============================================================================

//=============================================================================
NTSTATUS
CreateMiniportWaveCyclicMSVAD
( 
    OUT PUNKNOWN *              Unknown,
    IN  REFCLSID,
    IN  PUNKNOWN                UnknownOuter OPTIONAL,
    IN  POOL_TYPE               PoolType 
)
/*++

Routine Description:

  Create the wavecyclic miniport.

Arguments:

  Unknown - 

  RefClsId -

  UnknownOuter -

  PoolType -

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    ASSERT(Unknown);

    STD_CREATE_BODY(CMiniportWaveCyclic, Unknown, UnknownOuter, PoolType);
}

//=============================================================================
CMiniportWaveCyclic::~CMiniportWaveCyclic
( 
    void 

)
/*++

Routine Description:

  Destructor for wavecyclic miniport

Arguments:

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveCyclic::~CMiniportWaveCyclic]"));

	if (iSocket != 0)
	{
		iSocket->Close();
		ExFreePool(iSocket);
	}

	if (iWsk != 0)
	{
		iWsk->Close();
	}

} // ~CMiniportWaveCyclic


//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclic::DataRangeIntersection
( 
    IN  ULONG                       PinId,
    IN  PKSDATARANGE                ClientDataRange,
    IN  PKSDATARANGE                MyDataRange,
    IN  ULONG                       OutputBufferLength,
    OUT PVOID                       ResultantFormat,
    OUT PULONG                      ResultantFormatLength 
)
/*++

Routine Description:

  The DataRangeIntersection function determines the highest quality 
  intersection of two data ranges.

Arguments:

  PinId -           Pin for which data intersection is being determined. 

  ClientDataRange - Pointer to KSDATARANGE structure which contains the data 
                    range submitted by client in the data range intersection 
                    property request. 

  MyDataRange -         Pin's data range to be compared with client's data 
                        range. In this case we actually ignore our own data 
                        range, because we know that we only support one range.

  OutputBufferLength -  Size of the buffer pointed to by the resultant format 
                        parameter. 

  ResultantFormat -     Pointer to value where the resultant format should be 
                        returned. 

  ResultantFormatLength -   Actual length of the resultant format placed in 
                            ResultantFormat. This should be less than or equal 
                            to OutputBufferLength. 

  Return Value:

    NT status code.

--*/
{
    UNREFERENCED_PARAMETER(PinId);
    UNREFERENCED_PARAMETER(ClientDataRange);
    UNREFERENCED_PARAMETER(MyDataRange);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ResultantFormat);
    UNREFERENCED_PARAMETER(ResultantFormatLength);

	PAGED_CODE();

    // This driver only supports PCM formats.
    // Portcls will handle the request for us.
    //

    return STATUS_NOT_IMPLEMENTED;
} // DataRangeIntersection

//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclic::GetDescription
( 
    OUT PPCFILTER_DESCRIPTOR * OutFilterDescriptor 
)
/*++

Routine Description:

  The GetDescription function gets a pointer to a filter description. 
  It provides a location to deposit a pointer in miniport's description 
  structure. This is the placeholder for the FromNode or ToNode fields in 
  connections which describe connections to the filter's pins. 

Arguments:

  OutFilterDescriptor - Pointer to the filter description. 

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    return 
        CMiniportWaveCyclicMSVAD::GetDescription(OutFilterDescriptor);
} // GetDescription

//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclic::Init
( 
    IN  PUNKNOWN                UnknownAdapter_,
    IN  PRESOURCELIST           ResourceList_,
    IN  PPORTWAVECYCLIC         Port_ 
)
/*++

Routine Description:

  The Init function initializes the miniport. Callers of this function 
  should run at IRQL PASSIVE_LEVEL

Arguments:

  UnknownAdapter - A pointer to the IUnknown interface of the adapter object. 

  ResourceList - Pointer to the resource list to be supplied to the miniport 
                 during initialization. The port driver is free to examine the 
                 contents of the ResourceList. The port driver will not be 
                 modify the ResourceList contents. 

  Port - Pointer to the topology port object that is linked with this miniport. 

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    ASSERT(UnknownAdapter_);
    ASSERT(Port_);

    NTSTATUS                    ntStatus;

    DPF_ENTER(("[CMiniportWaveCyclic::Init]"));

    m_MaxOutputStreams      = MAX_OUTPUT_STREAMS;
    m_MaxInputStreams       = MAX_INPUT_STREAMS;
    m_MaxTotalStreams       = MAX_TOTAL_STREAMS;

    m_MinChannels           = MIN_CHANNELS;
    m_MaxChannelsPcm        = MAX_CHANNELS_PCM;

    m_MinBitsPerSamplePcm   = MIN_BITS_PER_SAMPLE_PCM;
    m_MaxBitsPerSamplePcm   = MAX_BITS_PER_SAMPLE_PCM;
    m_MinSampleRatePcm      = MIN_SAMPLE_RATE;
    m_MaxSampleRatePcm      = MAX_SAMPLE_RATE;
    
    ntStatus =
        CMiniportWaveCyclicMSVAD::Init
        (
            UnknownAdapter_,
            ResourceList_,
            Port_
        );

    if (NT_SUCCESS(ntStatus))
    {
        // Set filter descriptor.
        m_FilterDescriptor = &MiniportFilterDescriptor;

        iCaptureAllocated = false;
        iRenderAllocated = false;
    }

	iWsk = 0;
	iSocket = 0;

	iEnabled = 0;
	iActive = 0;
	iTtl = 0;
	iAddr = 0;
	iPort = 0;
	iLatency = 100;

	KeInitializeSpinLock(&iPipelineSpinLock);

	iPipelineMdl = 0;

	iPipelineSending = false;
	iPipelineStopped = true;

	iPipelinePacketBytes = 1828; // (441 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")

	iPipelineFrame = 0;

    iPipelinePerformanceCounter = KeQueryInterruptTime();

	iPipelineOutputBuf.Offset = 0;

	iPipelineHeader.iMagic[0] = 'O';
	iPipelineHeader.iMagic[1] = 'h';
	iPipelineHeader.iMagic[2] = 'm';
	iPipelineHeader.iMagic[3] = ' ';

	iPipelineHeader.iMajorVersion = 1;
	iPipelineHeader.iMsgType = 3;
	iPipelineHeader.iAudioHeaderBytes = 50;
	iPipelineHeader.iAudioFlags = 2; // lossless
	iPipelineHeader.iAudioSamples = 0;
	iPipelineHeader.iAudioFrame = 0;
	iPipelineHeader.iAudioNetworkTimestamp = 0;
	iPipelineHeader.iAudioMediaLatency = 0;
	iPipelineHeader.iAudioMediaTimestamp = 0;
	iPipelineHeader.iAudioSampleStartHi = 0;
	iPipelineHeader.iAudioSampleStartLo = 0;
	iPipelineHeader.iAudioSamplesTotalHi = 0;
	iPipelineHeader.iAudioSamplesTotalLo = 0;
	iPipelineHeader.iAudioSampleRate = 0;
	iPipelineHeader.iAudioBitRate = 0;
	iPipelineHeader.iAudioVolumeOffset = 0;
	iPipelineHeader.iAudioBitDepth = 0;
	iPipelineHeader.iAudioChannels = 0;
	iPipelineHeader.iReserved = 0;
	iPipelineHeader.iCodecNameBytes = 6;  // 3
	iPipelineHeader.iCodecName[0] = 'P';
	iPipelineHeader.iCodecName[1] = 'C';
	iPipelineHeader.iCodecName[2] = 'M';
	iPipelineHeader.iCodecName[3] = ' ';
	iPipelineHeader.iCodecName[4] = ' ';
	iPipelineHeader.iCodecName[5] = ' ';

	SetFormatLocked(44100, 1411200, 16, 2);

	CWinsock::Initialise(&iPipelineAddress, iAddr, iPort);

	iWsk = CWinsock::Create();

	if (iWsk == 0)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	iSocket = CSocketOhm::Create(*iWsk);

	if (iSocket == 0)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    return STATUS_SUCCESS;
}

//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclic::NewStream
( 
    OUT PMINIPORTWAVECYCLICSTREAM * OutStream,
    IN  PUNKNOWN                OuterUnknown,
    IN  POOL_TYPE               PoolType,
    IN  ULONG                   Pin,
    IN  BOOLEAN                 Capture,
    IN  PKSDATAFORMAT           DataFormat,
    OUT PDMACHANNEL *           OutDmaChannel,
    OUT PSERVICEGROUP *         OutServiceGroup 
)
/*++

Routine Description:

  The NewStream function creates a new instance of a logical stream 
  associated with a specified physical channel. Callers of NewStream should 
  run at IRQL PASSIVE_LEVEL.

Arguments:

  OutStream -

  OuterUnknown -

  PoolType - 

  Pin - 

  Capture - 

  DataFormat -

  OutDmaChannel -

  OutServiceGroup -

Return Value:

  NT status code.

--*/
{
    UNREFERENCED_PARAMETER(PoolType);

	PAGED_CODE();

    ASSERT(OutStream);
    ASSERT(DataFormat);
    ASSERT(OutDmaChannel);
    ASSERT(OutServiceGroup);

    DPF_ENTER(("[CMiniportWaveCyclic::NewStream]"));

    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PCMiniportWaveCyclicStream  stream = NULL;

    // Check if we have enough streams.
    if (Capture)
    {
        if (iCaptureAllocated)
        {
            DPF(D_TERSE, ("[Only one capture stream supported]"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        if (iRenderAllocated)
        {
            DPF(D_TERSE, ("[Only one render stream supported]"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    // Determine if the format is valid.
    //
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = ValidateFormat(DataFormat);
    }

    // Instantiate a stream. Stream must be in
    // NonPagedPool because of file saving.
    //
    if (NT_SUCCESS(ntStatus))
    {
        stream = new (NonPagedPool, OHSOUNDCARD_POOLTAG) 
            CMiniportWaveCyclicStream(OuterUnknown);

        if (stream)
        {
            stream->AddRef();

            ntStatus = 
                stream->Init
                ( 
                    this,
                    Pin,
                    Capture,
                    DataFormat
                );
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        if (Capture)
        {
            iCaptureAllocated = true;
        }
        else
        {
            iRenderAllocated = true;
        }

        *OutStream = PMINIPORTWAVECYCLICSTREAM(stream);
        (*OutStream)->AddRef();
        
        *OutDmaChannel = PDMACHANNEL(stream);
        (*OutDmaChannel)->AddRef();

        *OutServiceGroup = m_ServiceGroup;
        (*OutServiceGroup)->AddRef();

        // The stream, the DMA channel, and the service group have
        // references now for the caller.  The caller expects these
        // references to be there.
    }

    // This is our private reference to the stream.  The caller has
    // its own, so we can release in any case.
    //
    if (stream)
    {
        stream->Release();
    }
    
    return ntStatus;
} // NewStream

//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclic::NonDelegatingQueryInterface
( 
    IN  REFIID  Interface,
    OUT PVOID * Object 
)
/*++

Routine Description:

  QueryInterface

Arguments:

  Interface - GUID

  Object - interface pointer to be returned.

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVECYCLIC(this)));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportWaveCyclic))
    {
        *Object = PVOID(PMINIPORTWAVECYCLIC(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        // We reference the interface for the caller.

        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
} // NonDelegatingQueryInterface


//=============================================================================
// CMiniportWaveStreamCyclicSimple
//=============================================================================

//=============================================================================
CMiniportWaveCyclicStream::~CMiniportWaveCyclicStream
( 
    void 
)
/*++

Routine Description:

  Destructor for wavecyclicstream 

Arguments:

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveCyclicStream::~CMiniportWaveCyclicStream]"));

    if (NULL != m_pMiniportLocal)
    {
        if (m_fCapture)
        {
            m_pMiniportLocal->iCaptureAllocated = false;
        }
        else
        {
            m_pMiniportLocal->iRenderAllocated = false;
        }
    }
} // ~CMiniportWaveCyclicStream

//=============================================================================
NTSTATUS
CMiniportWaveCyclicStream::Init
( 
    IN PCMiniportWaveCyclic         Miniport_,
    IN ULONG                        Pin_,
    IN BOOLEAN                      Capture_,
    IN PKSDATAFORMAT                DataFormat_
)
/*++

Routine Description:

  Initializes the stream object. Allocate a DMA buffer, timer and DPC

Arguments:

  Miniport_ -

  Pin_ -

  Capture_ -

  DataFormat -

  DmaChannel_ -

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    m_pMiniportLocal = Miniport_;

    return 
        CMiniportWaveCyclicStreamMSVAD::Init
        (
            Miniport_,
            Pin_,
            Capture_,
            DataFormat_
        );
} // Init

//=============================================================================
STDMETHODIMP_(NTSTATUS)
CMiniportWaveCyclicStream::NonDelegatingQueryInterface
( 
    IN  REFIID  Interface,
    OUT PVOID * Object 
)
/*++

Routine Description:

  QueryInterface

Arguments:

  Interface - GUID

  Object - interface pointer to be returned

Return Value:

  NT status code.

--*/
{
	PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVECYCLICSTREAM(this)));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportWaveCyclicStream))
    {
        *Object = PVOID(PMINIPORTWAVECYCLICSTREAM(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IDmaChannel))
    {
        *Object = PVOID(PDMACHANNEL(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IDrmAudioStream))
    {
        *Object = (PVOID)(PDRMAUDIOSTREAM)this;
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
} // NonDelegatingQueryInterface

//=============================================================================
STDMETHODIMP_(NTSTATUS) 
CMiniportWaveCyclicStream::SetContentId
(
    IN  ULONG                   contentId,
    IN  PCDRMRIGHTS             drmRights
)
/*++

Routine Description:

  Sets DRM content Id for this stream. Also updates the Mixed content Id.

Arguments:

  contentId - new content id

  drmRights - rights for this stream.

Return Value:

  NT status code.

--*/
{
    UNREFERENCED_PARAMETER(contentId);
    UNREFERENCED_PARAMETER(drmRights);

	PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveCyclicStream::SetContentId]"));

    // if (drmRights.CopyProtect==TRUE)
    // Stop writing this stream to disk
    // 
    // if (drmRights.DigitalOutputDisable == TRUE)
    // Mute S/PDIF out. 
    // MSVAD does not support S/PDIF out. 
    //
    // To learn more about managing multiple streams, please look at MSVAD\drmmult
    //
    
	//m_SaveData.Disable(drmRights->CopyProtect);
    
    return STATUS_SUCCESS;
} // SetContentId

#pragma code_seg()

//=============================================================================
// UpdateEndpoint
//=============================================================================

void CMiniportWaveCyclic::UpdateEndpoint(TUint aAddress, TUint aPort, TUint aAdapter)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	CWinsock::Initialise(&iPipelineAddress, aAddress, aPort);

	if (iPipelineAdapter != aAdapter)
	{
		iPipelineAdapter = aAdapter;

		KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
	
		iSocket->SetMulticastIf(aAdapter);
	}
	else
	{
		KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
	}
}

//=============================================================================
// UpdateTtl
//=============================================================================

void CMiniportWaveCyclic::UpdateTtl(TUint aValue)
{
	iSocket->SetTtl(aValue);
}

//=============================================================================
// UpdateEnabled
//=============================================================================

void CMiniportWaveCyclic::UpdateEnabled(TUint aValue)
{
	TUint enabled = aValue;

	if (enabled != 0) {
		enabled = 1;
	}

	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iEnabled != enabled) {
		if (enabled) {
			iEnabled = 1;
		}
		else {
			iEnabled = 0;

			if (iActive && !iPipelineStopped) {
				// issue stop

				if (PipelineStopLocked())
				{
					if (!iPipelineSending)
					{
						iPipelineSending = true;

						KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

						PipelineOutput();

						return;
					}
				}
			}
		}
	}

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// UpdateActive
//=============================================================================

void CMiniportWaveCyclic::UpdateActive(TUint aValue)
{
	TUint active = aValue;

	if (active != 0) {
		active = 1;
	}

	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iActive != active) {
		if (active) {
			iActive = 1;
		}
		else {
			iActive = 0;

			if (iEnabled && !iPipelineStopped) {
				// issue stop

				if (PipelineStopLocked())
				{
					if (!iPipelineSending)
					{
						iPipelineSending = true;

						KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

						PipelineOutput();

						return;
					}
				}
			}
		}
	}

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// UpdateLatencyLocked
//=============================================================================

void CMiniportWaveCyclic::UpdateLatencyLocked()
{
	TUint multiplier = 48000 * 32; // divide by 125 for ms (256/1000 = 32/125)

	if ((iPipelineHeader.iAudioSampleRate % 441) == 0)
	{
		multiplier = 44100 * 32;
	}

	TUint latency = iLatency * multiplier / 125;

	iPipelineHeader.iAudioMediaLatency = latency >> 24 & 0x000000ff;
	iPipelineHeader.iAudioMediaLatency += latency >> 8 & 0x0000ff00;
	iPipelineHeader.iAudioMediaLatency += latency << 8 & 0x00ff0000;
	iPipelineHeader.iAudioMediaLatency += latency << 24 & 0xff000000;
}

//=============================================================================
// UpdateLatency
//=============================================================================

void CMiniportWaveCyclic::UpdateLatency(TUint aValue)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	iLatency = aValue;

	UpdateLatencyLocked();

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// MpusSetFormatLocked
//=============================================================================

void CMiniportWaveCyclic::SetFormatLocked(TUint aSampleRate, TUint aBitRate, TUint aBitDepth, TUint aChannels)
{
	iPipelineHeader.iAudioSampleRate = aSampleRate >> 24 & 0x000000ff;
	iPipelineHeader.iAudioSampleRate += aSampleRate >> 8 & 0x0000ff00;
	iPipelineHeader.iAudioSampleRate += aSampleRate << 8 & 0x00ff0000;
	iPipelineHeader.iAudioSampleRate += aSampleRate << 24 & 0xff000000;

	iPipelineHeader.iAudioBitRate = aBitRate >> 24 & 0x000000ff;
	iPipelineHeader.iAudioBitRate += aBitRate >> 8 & 0x0000ff00;
	iPipelineHeader.iAudioBitRate += aBitRate << 8 & 0x00ff0000;
	iPipelineHeader.iAudioBitRate += aBitRate << 24 & 0xff000000;

	iPipelineHeader.iAudioBitDepth = (UCHAR) aBitDepth;

	iPipelineHeader.iAudioChannels = (UCHAR) aChannels;

	iPipelineSampleChannelBytes = aBitDepth / 8;
	iPipelineSampleBytes = iPipelineSampleChannelBytes * aChannels;

	iPipelinePacketBytes = 1828; // (441 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")

	if (aSampleRate == 48000) {
		iPipelinePacketBytes = 1984; // (480 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")
	}

	UpdateLatencyLocked();
}

//=============================================================================
// SetFormat
//=============================================================================

void CMiniportWaveCyclic::SetFormat(TUint aSampleRate, TUint aBitRate, TUint aBitDepth, TUint aChannels)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iPipelineMdl != NULL)
	{
		PipelineQueueAddLocked(&iPipelineMdl, &iPipelineBytes); // queue old audio in the old format before applying the new format
	}

	SetFormatLocked(aSampleRate, aBitRate, aBitDepth, aChannels);

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// PipelineQueueAddLocked
//=============================================================================

// returns true if a message is added to the queue

TBool CMiniportWaveCyclic::PipelineQueueAddLocked(PMDL* aMdl, TUint* aBytes)
{
	if (iPipeline.SlotsUsed() == kMaxPipelineMessages) {
		PMDL mdl = *aMdl;

		*aMdl = NULL;
		*aBytes = 0;

		while (mdl != NULL)
		{
			ExFreePoolWithTag(MmGetMdlVirtualAddress(mdl), '2ten');
			PMDL next = mdl->Next;
			IoFreeMdl(mdl);
			mdl = next;
		}

		return (false);
	}

	OhmMsgAudio& msg = iFactory.CreateAudio(*aMdl, *aBytes);

	iPipeline.Write(&msg);

	*aMdl = NULL;
	*aBytes = 0;

	return (true);
}

//=============================================================================
// PipelineQueueRemove
//=============================================================================

OhmMsgAudio* CMiniportWaveCyclic::PipelineQueueRemove(SOCKADDR* aAddress)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iPipeline.SlotsUsed() == 0)
	{
		iPipelineSending = false;

		KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

		return (0);
	}

	OhmMsgAudio* msg = iPipeline.Read();

	if (!msg->Resent()) {
		if (iHistory.SlotsUsed() == kMaxPipelineMessages) {
			iHistory.Read()->RemoveRef(); // discard
		}

		iHistory.Write(msg);

		msg->AddRef();
	}

	RtlCopyMemory(aAddress, &iPipelineAddress, sizeof(SOCKADDR));

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

	return (msg);
}

//=============================================================================
// PipelineOutput
//=============================================================================

void CMiniportWaveCyclic::PipelineOutput()
{
	iPipelineOutputMsg = PipelineQueueRemove(&iPipelineOutputAddress);

	if (iPipelineOutputMsg == 0)
	{
		return;
	}

	if (!iPipelineOutputMsg->Resent()) {
		TUint bytes = iPipelineOutputMsg->Bytes();

		// Fill in multipus header

		OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(iPipelineOutputMsg->Mdl());

		// samples

		TUint audioBytes = bytes - sizeof(OHMHEADER);

		TUint sampleBytes = header->iAudioChannels * header->iAudioBitDepth / 8;

		TUint samples = audioBytes / sampleBytes;

		header->iAudioSamples = samples >> 8 & 0x00ff;
		header->iAudioSamples += samples << 8 & 0xff00;

		// frame

		TUint frame = ++iPipelineFrame;

		header->iAudioFrame = frame >> 24 & 0x000000ff;
		header->iAudioFrame += frame >> 8 & 0x0000ff00;
		header->iAudioFrame += frame << 8 & 0x00ff0000;
		header->iAudioFrame += frame << 24 & 0xff000000;

		// bytes

		header->iTotalBytes = bytes >> 8 & 0x00ff;
		header->iTotalBytes += bytes << 8 & 0xff00;

		// Create Timestamps

		TUint multiplier = 48000 * 256;

		if ((header->iAudioSampleRate % 441) == 0)
		{
			multiplier = 44100 * 256;
		}

		// use last timestamp for this message

		TUint64 timestamp = iPipelinePerformanceCounter;

		timestamp *= multiplier;
		timestamp /= 10000000; // InterruptTime is in 100ns units

		header->iAudioNetworkTimestamp = timestamp >> 24 & 0x000000ff;
		header->iAudioNetworkTimestamp += timestamp >> 8 & 0x0000ff00;
		header->iAudioNetworkTimestamp += timestamp << 8 & 0x00ff0000;
		header->iAudioNetworkTimestamp += timestamp << 24 & 0xff000000;

		header->iAudioMediaTimestamp = header->iAudioNetworkTimestamp;
	}

	iPipelineOutputBuf.Mdl = iPipelineOutputMsg->Mdl();
	iPipelineOutputBuf.Length = iPipelineOutputMsg->Bytes();

	PIRP irp = IoAllocateIrp(1, FALSE);
	
	IoSetCompletionRoutine(irp, PipelineOutputComplete, this, true, true, true);

	iSocket->Send(&iPipelineOutputBuf, &iPipelineOutputAddress, irp);
}

//=============================================================================
// PipelineOutputComplete
//=============================================================================

NTSTATUS CMiniportWaveCyclic::PipelineOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
	CMiniportWaveCyclic* miniport = (CMiniportWaveCyclic*) aContext;
	return (miniport->PipelineOutputComplete(aDeviceObject, aIrp));
}

NTSTATUS CMiniportWaveCyclic::PipelineOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp)
{
    UNREFERENCED_PARAMETER(aDeviceObject);

	// get timestamp for next message

    iPipelinePerformanceCounter = KeQueryInterruptTime();

	iPipelineOutputMsg->SetResent(true);
	iPipelineOutputMsg->RemoveRef();

	IoFreeIrp(aIrp);

	PipelineOutput(); // send the next message if there is one

	// Always return STATUS_MORE_PROCESSING_REQUIRED to
	// terminate the completion processing of the IRP.

	return STATUS_MORE_PROCESSING_REQUIRED;
}

//=============================================================================
// PipelineSendNewLocked
//=============================================================================

void CMiniportWaveCyclic::PipelineSendNewLocked()
{
	void* header = ExAllocatePoolWithTag(NonPagedPool, sizeof(OHMHEADER), '2ten');

	RtlCopyMemory(header, &iPipelineHeader, sizeof(OHMHEADER));
	
	PMDL mdl = IoAllocateMdl(header, sizeof(OHMHEADER), false, false, 0);

	MmBuildMdlForNonPagedPool(mdl);

	iPipelineMdl = mdl;
	iPipelineMdlLast = mdl;

	iPipelineBytes = sizeof(OHMHEADER);
}

//=============================================================================
// PipelineCopyAudioLocked
//=============================================================================

void CMiniportWaveCyclic::PipelineCopyAudioLocked(TByte* aDestination, TByte* aSource, TUint aBytes, TUint aSampleBytes)
{
	TByte* dst = aDestination;
	TByte* src = aSource;
	TUint bytes = aBytes;
	TUint sb = aSampleBytes;

	while (bytes > 0)
	{
		UCHAR* s = src + sb;

		while (s > src)
		{
			*dst++ = *--s;
		}

		src += sb;
		bytes -= sb;
	}
}

//=============================================================================
// PipelineSendAddFragmentLocked
//=============================================================================

void CMiniportWaveCyclic::PipelineSendAddFragmentLocked(TByte* aBuffer, TUint aBytes)
{
	TByte* buffer = (TByte*) ExAllocatePoolWithTag(NonPagedPool, aBytes, '2ten');

	PMDL mdl = IoAllocateMdl(buffer, aBytes, FALSE, FALSE, NULL);

	MmBuildMdlForNonPagedPool(mdl);

	iPipelineMdlLast->Next = mdl;
	iPipelineMdlLast = mdl;

	iPipelineBytes += aBytes;

	PipelineCopyAudioLocked(buffer, aBuffer, aBytes, iPipelineSampleChannelBytes);
}

//=============================================================================
// PipelineSendLocked
//=============================================================================

// returns true if a message was added to the queue

TBool CMiniportWaveCyclic::PipelineSendLocked(TByte* aBuffer, TUint aBytes)
{
	iPipelineStopped = false;

	if (iPipelineMdl == 0)
	{
		PipelineSendNewLocked();
	}

	TUint combined = iPipelineBytes + aBytes;

	if (combined < iPipelinePacketBytes)
	{
		PipelineSendAddFragmentLocked(aBuffer, aBytes);
		return (false);
	}

	TUint first = iPipelinePacketBytes - iPipelineBytes;

	PipelineSendAddFragmentLocked(aBuffer, first);

	bool result = PipelineQueueAddLocked(&iPipelineMdl, &iPipelineBytes);

	TUint remaining = combined - iPipelinePacketBytes;

	if (remaining == 0)
	{
		return (true);
	}

	PipelineSendNewLocked();

	PipelineSendAddFragmentLocked(aBuffer + first, remaining);

	return (result);
}

//=============================================================================
// PipelineSend
//=============================================================================

void CMiniportWaveCyclic::PipelineSend(TByte* aBuffer, TUint aBytes)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iActive && iEnabled)
	{
		if (PipelineSendLocked(aBuffer, aBytes))
		{
			if (!iPipelineSending)
			{
				iPipelineSending = true;

				KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

				PipelineOutput();

				return;
			}
		}
	}

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// PipelineStopLocked
//=============================================================================

// return true if PipelineOutput has to be called to send this stop message

TBool CMiniportWaveCyclic::PipelineStopLocked()
{
	iPipelineStopped = true;

	if (iPipelineMdl == 0)
	{
		PipelineSendNewLocked();

		TByte silence[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		PipelineSendAddFragmentLocked(silence, iPipelineSampleBytes);
	}

	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(iPipelineMdl);

	header->iAudioFlags = 3; // lossless + halt

	return (PipelineQueueAddLocked(&iPipelineMdl, &iPipelineBytes));
}

//=============================================================================
// PipelineStop
//=============================================================================

void CMiniportWaveCyclic::PipelineStop()
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	if (iActive && iEnabled && !iPipelineStopped)
	{
		if (PipelineStopLocked())
		{
			if (!iPipelineSending)
			{
				iPipelineSending = true;

				KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

				PipelineOutput();

				return;
			}
		}
	}

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

//=============================================================================
// Resend
//=============================================================================

TBool CMiniportWaveCyclic::ResendLocked(OhmMsgAudio& aMsg)
{
	if (iPipeline.SlotsUsed() != kMaxPipelineMessages) {
		aMsg.AddRef();
		iPipeline.Write(&aMsg);
		return (true);
	}

	return (false);
}

//=============================================================================
// PipelineResend
//=============================================================================

void CMiniportWaveCyclic::PipelineResend(const Brx& aFrames)
{
	TUint resent = 0;

	KIRQL oldIrql;

	KeAcquireSpinLock(&iPipelineSpinLock, &oldIrql);

	ReaderBuffer buffer(aFrames);
	ReaderBinary reader(buffer);

	TUint frames = aFrames.Bytes() / 4;

	TUint frame = reader.ReadUintBe(4);

	frames--;

	TBool found = false;

	TUint count = iHistory.SlotsUsed();

	for (TUint i = 0; i < count; i++) {
		OhmMsgAudio* msg = iHistory.Read();

		if (!found) {
			TInt diff = frame - msg->Frame();

			if (diff == 0) {
				if (ResendLocked(*msg)) {
					resent++;
				}
				if (frames-- > 0) {
					frame = reader.ReadUintBe(4);
				}
				else {
					found = true;
				}
			}
			else {
				while (diff < 0) {
					if (frames-- > 0) {
						frame = reader.ReadUintBe(4);
					}
					else {
						found = true;
						break;
					}

					diff = frame - msg->Frame();

					if (diff == 0) {
						if (ResendLocked(*msg)) {
							resent++;
						}
						if (frames-- > 0) {
							frame = reader.ReadUintBe(4);
						}
						else {
							found = true;
						}

						break;
					}
				}
			}
		}

		iHistory.Write(msg);
	}

	if (resent > 0) {
		if (!iPipelineSending)
		{
			iPipelineSending = true;

			KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);

			PipelineOutput();

			return;
		}
	}

	KeReleaseSpinLock(&iPipelineSpinLock, oldIrql);
}

