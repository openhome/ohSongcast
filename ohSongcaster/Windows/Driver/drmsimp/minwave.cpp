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

KSPIN_LOCK MpusSpinLock;

UINT MpusEnabled;
UINT MpusActive;
UINT MpusTtl;
UINT MpusAddr;
UINT MpusPort;
UINT MpusLatency;

UINT MpusPacketBytes;
UINT MpusSampleBytes;
UINT MpusSampleChannelBytes;

bool MpusSending;
bool MpusStopped;

PMDL MpusMdl;
PMDL MpusMdlLast;

UINT MpusAdapter;
SOCKADDR MpusAddress;
SOCKADDR MpusOutputAddress;

UINT MpusFrame;

ULONGLONG MpusPerformanceCounter;

WSK_BUF MpusSendBuf;

KEVENT WskInitialisedEvent;

OHMHEADER MpusHeader;

ULONG MpusBytes;

CWinsock* Wsk;
CSocketOhm* Socket;

void SocketInitialised(void* aContext);

void MpusOutput();
void MpusSetFormatLocked(UINT aSampleRate, UINT aBitRate, UINT aBitDepth, UINT aChannels);

struct MpusQueueEntry
{
	PMDL iMdl;
	ULONG iBytes;
	SOCKADDR iAddress;
};

ULONG MpusQueueCount;
ULONG MpusQueueIndexRead;
ULONG MpusQueueIndexWrite;

MpusQueueEntry MpusQueue[16];

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

	if (Socket != NULL)
	{
		Socket->Close();
		ExFreePool(Socket);
	}

	if (Wsk != NULL)
	{
		Wsk->Close();
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

        m_fCaptureAllocated = FALSE;
        m_fRenderAllocated = FALSE;
    }

	MpusEnabled = 0;
	MpusActive = 0;
	MpusTtl = 0;
	MpusAddr = 0;
	MpusPort = 0;
	MpusLatency = 100;

	KeInitializeSpinLock(&MpusSpinLock);

	MpusMdl = NULL;

	MpusSending = false;
	MpusStopped = true;

	MpusPacketBytes = 1828; // (441 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")

	MpusFrame = 0;

    MpusPerformanceCounter = KeQueryInterruptTime();

	MpusQueueCount = 0;
	MpusQueueIndexRead = 0;
	MpusQueueIndexWrite = 0;

	MpusSendBuf.Offset = 0;

	MpusHeader.iMagic[0] = 'O';
	MpusHeader.iMagic[1] = 'h';
	MpusHeader.iMagic[2] = 'm';
	MpusHeader.iMagic[3] = ' ';

	MpusHeader.iMajorVersion = 1;
	MpusHeader.iMsgType = 3;
	MpusHeader.iAudioHeaderBytes = 50;
	MpusHeader.iAudioFlags = 2; // lossless
	MpusHeader.iAudioSamples = 0;
	MpusHeader.iAudioFrame = 0;
	MpusHeader.iAudioNetworkTimestamp = 0;
	MpusHeader.iAudioMediaLatency = 0;
	MpusHeader.iAudioMediaTimestamp = 0;
	MpusHeader.iAudioSampleStartHi = 0;
	MpusHeader.iAudioSampleStartLo = 0;
	MpusHeader.iAudioSamplesTotalHi = 0;
	MpusHeader.iAudioSamplesTotalLo = 0;
	MpusHeader.iAudioSampleRate = 0;
	MpusHeader.iAudioBitRate = 0;
	MpusHeader.iAudioVolumeOffset = 0;
	MpusHeader.iAudioBitDepth = 0;
	MpusHeader.iAudioChannels = 0;
	MpusHeader.iReserved = 0;
	MpusHeader.iCodecNameBytes = 6;  // 3
	MpusHeader.iCodecName[0] = 'P';
	MpusHeader.iCodecName[1] = 'C';
	MpusHeader.iCodecName[2] = 'M';
	MpusHeader.iCodecName[3] = ' ';
	MpusHeader.iCodecName[4] = ' ';
	MpusHeader.iCodecName[5] = ' ';

	MpusSetFormatLocked(44100, 1411200, 16, 2);

	Wsk = NULL;
	Socket = NULL;

	CWinsock::Initialise(&MpusAddress, MpusAddr, MpusPort);

	KeInitializeEvent(&WskInitialisedEvent, SynchronizationEvent, false);

	Wsk = CWinsock::Create();

	if (Wsk == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Socket = new (NonPagedPool, OHSOUNDCARD_POOLTAG) CSocketOhm();

	Socket->Initialise(*Wsk, SocketInitialised, this);

	LARGE_INTEGER timeout;

	timeout.QuadPart = -1200000000; // 120 seconds

	ntStatus = KeWaitForSingleObject(&WskInitialisedEvent, Executive, KernelMode, false, &timeout);

	if(ntStatus != STATUS_SUCCESS)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    return ntStatus;
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
        if (m_fCaptureAllocated)
        {
            DPF(D_TERSE, ("[Only one capture stream supported]"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        if (m_fRenderAllocated)
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
            m_fCaptureAllocated = TRUE;
        }
        else
        {
            m_fRenderAllocated = TRUE;
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
            m_pMiniportLocal->m_fCaptureAllocated = FALSE;
        }
        else
        {
            m_pMiniportLocal->m_fRenderAllocated = FALSE;
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
// SocketInitialised
//=============================================================================

void SocketInitialised(void* aContext)
{
    UNREFERENCED_PARAMETER(aContext);

	KeSetEvent(&WskInitialisedEvent, 0, false);
}

//=============================================================================
// MpusQueueAddLocked
//=============================================================================

// returns true if a message is added to the queue

bool MpusQueueAddLocked(PMDL* aMdl, ULONG* aBytes, SOCKADDR* aAddress)
{
	if (++MpusQueueCount > 16)
	{
		--MpusQueueCount;

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

	MpusQueue[MpusQueueIndexWrite].iMdl = *aMdl;
	MpusQueue[MpusQueueIndexWrite].iBytes = *aBytes;
	RtlCopyMemory(&(MpusQueue[MpusQueueIndexWrite].iAddress), aAddress, sizeof(SOCKADDR));

	if (++MpusQueueIndexWrite >= 16)
	{
		MpusQueueIndexWrite = 0;
	}

	*aMdl = NULL;
	*aBytes = 0;

	return (true);
}


//=============================================================================
// MpusQueueRemove
//=============================================================================

bool MpusQueueRemove(PMDL* aMdl, ULONG* aBytes, SOCKADDR* aAddress)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusQueueCount == 0)
	{
		MpusSending = false;
		KeReleaseSpinLock(&MpusSpinLock, oldIrql);
		return (false);
	}

	*aMdl = MpusQueue[MpusQueueIndexRead].iMdl;
	*aBytes = MpusQueue[MpusQueueIndexRead].iBytes;
	RtlCopyMemory(aAddress, &(MpusQueue[MpusQueueIndexRead].iAddress), sizeof(SOCKADDR));

	if (++MpusQueueIndexRead >= 16)
	{
		MpusQueueIndexRead = 0;
	}

	MpusQueueCount--;

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);

	return (true);
}

//=============================================================================
// MpusUpdateEndpoint
//=============================================================================

void MpusUpdateEndpoint(UINT aAddress, UINT aPort, UINT aAdapter)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	CWinsock::Initialise(&MpusAddress, aAddress, aPort);

	MpusAdapter = aAdapter;

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusUpdateTtl
//=============================================================================

void MpusUpdateTtl(UINT aValue)
{
	Socket->SetTtl(aValue);
}

// Forward declaration

NTSTATUS MpusOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext);

//=============================================================================
// MpusOutput
//=============================================================================

void MpusOutput()
{
	ULONG bytes;

	if (!MpusQueueRemove(&MpusSendBuf.Mdl, &bytes, &MpusOutputAddress))
	{
		return;
	}

	MpusSendBuf.Length = bytes;

	// Fill in multipus header

	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(MpusSendBuf.Mdl);

	// samples

	ULONG audioBytes = bytes - sizeof(OHMHEADER);

	ULONG sampleBytes = header->iAudioChannels * header->iAudioBitDepth / 8;

	USHORT samples = (USHORT)(audioBytes / sampleBytes);

	header->iAudioSamples = samples >> 8 & 0x00ff;
	header->iAudioSamples += samples << 8 & 0xff00;

	// frame

	ULONG frame = ++MpusFrame;

	header->iAudioFrame = frame >> 24 & 0x000000ff;
	header->iAudioFrame += frame >> 8 & 0x0000ff00;
	header->iAudioFrame += frame << 8 & 0x00ff0000;
	header->iAudioFrame += frame << 24 & 0xff000000;

	// bytes

	header->iTotalBytes = bytes >> 8 & 0x00ff;
	header->iTotalBytes += bytes << 8 & 0xff00;

	// Create Timestamps

	ULONG multiplier = 48000 * 256;

	if ((header->iAudioSampleRate % 441) == 0)
	{
		multiplier = 44100 * 256;
	}

	// use last timestamp for this message

	ULONGLONG timestamp = MpusPerformanceCounter;

	timestamp *= multiplier;
	timestamp /= 10000000; // InterruptTime is in 100ns units

	header->iAudioNetworkTimestamp = timestamp >> 24 & 0x000000ff;
	header->iAudioNetworkTimestamp += timestamp >> 8 & 0x0000ff00;
	header->iAudioNetworkTimestamp += timestamp << 8 & 0x00ff0000;
	header->iAudioNetworkTimestamp += timestamp << 24 & 0xff000000;

	header->iAudioMediaTimestamp = header->iAudioNetworkTimestamp;

	PIRP irp = IoAllocateIrp(1, FALSE);
	
	IoSetCompletionRoutine(irp, MpusOutputComplete, NULL, TRUE, TRUE, TRUE);

	Socket->Send(&MpusSendBuf, &MpusOutputAddress, irp);
}

//=============================================================================
// MpusOutputComplete
//=============================================================================

NTSTATUS MpusOutputComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
    UNREFERENCED_PARAMETER(aDeviceObject);
    UNREFERENCED_PARAMETER(aContext);

	// get timestamp for next message

    MpusPerformanceCounter = KeQueryInterruptTime();

	PMDL mdl = MpusSendBuf.Mdl;

	while (mdl != NULL)
	{
		ExFreePoolWithTag(MmGetMdlVirtualAddress(mdl), '2ten');
		PMDL next = mdl->Next;
		IoFreeMdl(mdl);
		mdl = next;
	}

	IoFreeIrp(aIrp);

	MpusOutput(); // send the next message if there is one

	// Always return STATUS_MORE_PROCESSING_REQUIRED to
	// terminate the completion processing of the IRP.

	return STATUS_MORE_PROCESSING_REQUIRED;
}


//=============================================================================
// MpusSendNewLocked
//=============================================================================

void MpusSendNewLocked()
{
	void* header = ExAllocatePoolWithTag(NonPagedPool, sizeof(OHMHEADER), '2ten');

	RtlCopyMemory(header, &MpusHeader, sizeof(OHMHEADER));
	
	PMDL mdl = IoAllocateMdl(header, sizeof(OHMHEADER), FALSE, FALSE, NULL);

	MmBuildMdlForNonPagedPool(mdl);

	MpusMdl = mdl;
	MpusMdlLast = mdl;

	MpusBytes = sizeof(OHMHEADER);
}

//=============================================================================
// MpusCopyAudio
//=============================================================================

void MpusCopyAudioLocked(UCHAR* aDestination, UCHAR* aSource, UINT aBytes, UINT aSampleBytes)
{
	UCHAR* dst = aDestination;
	UCHAR* src = aSource;
	UINT bytes = aBytes;
	UINT sb = aSampleBytes;

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
// MpusSendAddFragmentLocked
//=============================================================================

void MpusSendAddFragmentLocked(UCHAR* aBuffer, UINT aBytes)
{
	UCHAR* buffer = (UCHAR*) ExAllocatePoolWithTag(NonPagedPool, aBytes, '2ten');

	PMDL mdl = IoAllocateMdl(buffer, aBytes, FALSE, FALSE, NULL);

	MmBuildMdlForNonPagedPool(mdl);

	MpusMdlLast->Next = mdl;
	MpusMdlLast = mdl;

	MpusBytes += aBytes;

	// copy audio

	MpusCopyAudioLocked(buffer, aBuffer, aBytes, MpusSampleChannelBytes);
}

//=============================================================================
// MpusSendLocked
//=============================================================================

// returns true if a message was added to the queue

bool MpusSendLocked(UCHAR* aBuffer, UINT aBytes)
{
	MpusStopped = false;

	if (MpusMdl == NULL)
	{
		MpusSendNewLocked();
	}

	UINT combined = MpusBytes + aBytes;

	if (combined < MpusPacketBytes)
	{
		MpusSendAddFragmentLocked(aBuffer, aBytes);

		return (false);
	}

	UINT first = MpusPacketBytes - MpusBytes;

	MpusSendAddFragmentLocked(aBuffer, first);

	bool result = MpusQueueAddLocked(&MpusMdl, &MpusBytes, &MpusAddress);

	UINT remaining = combined - MpusPacketBytes;

	if (remaining == 0)
	{
		return (true);
	}

	MpusSendNewLocked();

	MpusSendAddFragmentLocked(aBuffer + first, remaining);

	return (result);
}

//=============================================================================
// MpusSend
//=============================================================================

void MpusSend(UCHAR* aBuffer, UINT aBytes)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusActive && MpusEnabled)
	{
		if (MpusSendLocked(aBuffer, aBytes))
		{
			if (!MpusSending)
			{
				MpusSending = true;

				KeReleaseSpinLock(&MpusSpinLock, oldIrql);

				MpusOutput();

				return;
			}
		}
	}

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusSendStopLocked
//=============================================================================

// return true if MpusOutput has to be called to send this stop message

bool MpusStopLocked()
{
	MpusStopped = true;

	if (MpusMdl == NULL)
	{
		MpusSendNewLocked();

		UCHAR silence[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		MpusSendAddFragmentLocked(silence, MpusSampleBytes);
	}

	OHMHEADER* header = (OHMHEADER*) MmGetMdlVirtualAddress(MpusMdl);

	header->iAudioFlags = 3; // lossless + halt

	return (MpusQueueAddLocked(&MpusMdl, &MpusBytes, &MpusAddress));
}

//=============================================================================
// MpusStop
//=============================================================================

void MpusStop()
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusActive && MpusEnabled && !MpusStopped)
	{
		if (MpusStopLocked())
		{
			if (!MpusSending)
			{
				MpusSending = true;

				KeReleaseSpinLock(&MpusSpinLock, oldIrql);

				MpusOutput();

				return;
			}
		}
	}

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusUpdateEnabled
//=============================================================================

void MpusUpdateEnabled(UINT aValue)
{
	UINT enabled = aValue;

	if (enabled != 0) {
		enabled = 1;
	}

	// PCMiniportTopology  pMiniport = (PCMiniportTopology)PropertyRequest->MajorTarget;

	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusEnabled != enabled) {
		if (enabled) {
			MpusEnabled = 1;
		}
		else {
			MpusEnabled = 0;

			if (MpusActive && !MpusStopped) {
				// issue stop

				if (MpusStopLocked())
				{
					if (!MpusSending)
					{
						MpusSending = true;

						KeReleaseSpinLock(&MpusSpinLock, oldIrql);

						MpusOutput();

						return;
					}
				}
			}
		}
	}

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusUpdateActive
//=============================================================================

void MpusUpdateActive(UINT aValue)
{
	UINT active = aValue;

	if (active != 0) {
		active = 1;
	}

	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusActive != active) {
		if (active) {
			MpusActive = 1;
		}
		else {
			MpusActive = 0;

			if (MpusEnabled && !MpusStopped) {
				// issue stop

				if (MpusStopLocked())
				{
					if (!MpusSending)
					{
						MpusSending = true;

						KeReleaseSpinLock(&MpusSpinLock, oldIrql);

						MpusOutput();

						return;
					}
				}
			}
		}
	}

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusUpdateLatencyLocked
//=============================================================================

void MpusUpdateLatencyLocked()
{
	ULONG multiplier = 48000 * 32; // divide by 125 for ms (256/1000 = 32/125)

	if ((MpusHeader.iAudioSampleRate % 441) == 0)
	{
		multiplier = 44100 * 32;
	}

	ULONG latency = MpusLatency * multiplier / 125;

	MpusHeader.iAudioMediaLatency = latency >> 24 & 0x000000ff;
	MpusHeader.iAudioMediaLatency += latency >> 8 & 0x0000ff00;
	MpusHeader.iAudioMediaLatency += latency << 8 & 0x00ff0000;
	MpusHeader.iAudioMediaLatency += latency << 24 & 0xff000000;
}

//=============================================================================
// MpusUpdateLatency
//=============================================================================

void MpusUpdateLatency(UINT aValue)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	MpusLatency = aValue;

	MpusUpdateLatencyLocked();

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

//=============================================================================
// MpusSetFormatLocked
//=============================================================================

void MpusSetFormatLocked(UINT aSampleRate, UINT aBitRate, UINT aBitDepth, UINT aChannels)
{
	MpusHeader.iAudioSampleRate = aSampleRate >> 24 & 0x000000ff;
	MpusHeader.iAudioSampleRate += aSampleRate >> 8 & 0x0000ff00;
	MpusHeader.iAudioSampleRate += aSampleRate << 8 & 0x00ff0000;
	MpusHeader.iAudioSampleRate += aSampleRate << 24 & 0xff000000;

	MpusHeader.iAudioBitRate = aBitRate >> 24 & 0x000000ff;
	MpusHeader.iAudioBitRate += aBitRate >> 8 & 0x0000ff00;
	MpusHeader.iAudioBitRate += aBitRate << 8 & 0x00ff0000;
	MpusHeader.iAudioBitRate += aBitRate << 24 & 0xff000000;

	MpusHeader.iAudioBitDepth = (UCHAR) aBitDepth;

	MpusHeader.iAudioChannels = (UCHAR) aChannels;

	MpusSampleChannelBytes = aBitDepth / 8;
	MpusSampleBytes = MpusSampleChannelBytes * aChannels;

	MpusPacketBytes = 1828; // (441 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")

	if (aSampleRate == 48000) {
		MpusPacketBytes = 1984; // (480 * 2 channels * 2 bytes) + 50 Audio Header + 8 Mpus Header + 6 Codec Name ("PCM   ")
	}

	MpusUpdateLatencyLocked();
}

//=============================================================================
// MpusSetFormat
//=============================================================================

void MpusSetFormat(UINT aSampleRate, UINT aBitRate, UINT aBitDepth, UINT aChannels)
{
	KIRQL oldIrql;

	KeAcquireSpinLock(&MpusSpinLock, &oldIrql);

	if (MpusMdl != NULL)
	{
		MpusQueueAddLocked(&MpusMdl, &MpusBytes, &MpusAddress); // queue audio in the old format so it doesn't get audio in the new format added to it
	}

	MpusSetFormatLocked(aSampleRate, aBitRate, aBitDepth, aChannels);

	KeReleaseSpinLock(&MpusSpinLock, oldIrql);
}

