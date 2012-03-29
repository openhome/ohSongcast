/*
*******************************************************************************
**    Copyright (c) 1998-2000 Microsoft Corporation. All Rights Reserved.
**
**       Portions Copyright (c) 1998-1999 Intel Corporation
**
*******************************************************************************
*/

// Every debug output has "Modulname text"
static char STR_MODULENAME[] = "OHSOUNDCARD Network: ";

#include "network.h"
//#include "wdm.h"

#if (NTDDI_VERSION >= NTDDI_VISTA)

// CWinsock

void CWinsock::Initialise(PSOCKADDR aSocket, ULONG aAddress, ULONG aPort)
{
	SOCKADDR_IN* addr = (SOCKADDR_IN*) aSocket;

	addr->sin_family = AF_INET;
	addr->sin_port = ((aPort >> 8) & 0x00ff) + ((aPort << 8) & 0xff00);
	addr->sin_addr.S_un.S_un_b.s_b1 = (aAddress >> 24) & 0xff;
	addr->sin_addr.S_un.S_un_b.s_b2 = (aAddress >> 16) & 0xff;
	addr->sin_addr.S_un.S_un_b.s_b3 = (aAddress >> 8) & 0xff;
	addr->sin_addr.S_un.S_un_b.s_b4 = (aAddress & 0xff);
	addr->sin_zero[0] = 0;
	addr->sin_zero[1] = 0;
	addr->sin_zero[2] = 0;
	addr->sin_zero[3] = 0;
	addr->sin_zero[4] = 0;
	addr->sin_zero[5] = 0;
	addr->sin_zero[6] = 0;
	addr->sin_zero[7] = 0;
}

void CWinsock::Initialise(PSOCKADDR aSocket)
{
	Initialise(aSocket, 0, 0);
}

CWinsock* CWinsock::Create()
{
	CWinsock* winsock = (CWinsock*) ExAllocatePoolWithTag(NonPagedPool, sizeof(CWinsock), '1ten');

	if (winsock == 0) {
		return (0);
	}

	KeInitializeEvent(&winsock->iInitialised, SynchronizationEvent, false);

	winsock->iAppDispatch.Version = MAKE_WSK_VERSION(1,0);
	winsock->iAppDispatch.Reserved = 0;
	winsock->iAppDispatch.WskClientEvent = NULL;

	// Register as a WSK application

	WSK_CLIENT_NPI clientNpi;

	clientNpi.ClientContext = winsock;
	clientNpi.Dispatch = &winsock->iAppDispatch;

	NTSTATUS status = WskRegister(&clientNpi, &winsock->iRegistration);

	if(status == STATUS_SUCCESS)
	{
		HANDLE handle;

		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

		status = PsCreateSystemThread(&handle, THREAD_ALL_ACCESS, &oa, NULL, NULL, Init, winsock);

		if (status == STATUS_SUCCESS)
		{
			LARGE_INTEGER timeout;

			timeout.QuadPart = -1200000000; // 120 seconds in 100nS units

			status = KeWaitForSingleObject(&winsock->iInitialised, Executive, KernelMode, false, &timeout);

			ZwClose(handle);

			if(status == STATUS_SUCCESS)
			{
				return (winsock);
			}
		}

		WskDeregister(&winsock->iRegistration);
	}

	ExFreePoolWithTag(winsock, '1ten');

	return (0);
}

void CWinsock::Init(void* aContext)
{
	CWinsock* winsock = (CWinsock*)aContext;

	NTSTATUS status;

	// Capture the WSK Provider NPI. If WSK subsystem is not ready yet, wait until it becomes ready.

	status = WskCaptureProviderNPI(&(winsock->iRegistration), WSK_INFINITE_WAIT, &(winsock->iProviderNpi));

	if (status == STATUS_SUCCESS)
	{
		// Indicate that we are initialised

		KeSetEvent(&winsock->iInitialised, 0, false);
	}
}

void CWinsock::Close()
{
	WskReleaseProviderNPI(&iRegistration);

	WskDeregister(&iRegistration);

	ExFreePoolWithTag(this, '1ten');
}

// CSocketOhm

CSocketOhm* CSocketOhm::Create(CWinsock& aWsk)
{
	CSocketOhm* socket = (CSocketOhm*) ExAllocatePoolWithTag(NonPagedPool, sizeof(CSocketOhm), '1ten');

	if (socket == 0) {
		return (0);
	}

	socket->iWsk = &aWsk;

	KeInitializeEvent(&socket->iInitialised, SynchronizationEvent, false);

	// Create the socket

	NTSTATUS status;

	PIRP irp;

	// Allocate an IRP
	irp = IoAllocateIrp(1, FALSE);

	// Check result

	if (irp == 0) {
		ExFreePoolWithTag(socket, '1ten');
        return 0;
    }

	// Set the completion routine for the IRP
	IoSetCompletionRoutine(irp,	CreateComplete, socket, TRUE, TRUE, TRUE);

	// Initiate the creation of the socket

	status = aWsk.iProviderNpi.Dispatch->WskSocket(
		  aWsk.iProviderNpi.Client,
		  AF_INET,
		  SOCK_DGRAM,
		  IPPROTO_UDP,
		  WSK_FLAG_DATAGRAM_SOCKET,
		  socket,
		  NULL,
		  NULL,
		  NULL,
		  NULL,
		  irp
		  );

	if (status == STATUS_SUCCESS || status == STATUS_PENDING)
	{
		LARGE_INTEGER timeout;

		timeout.QuadPart = -1200000000; // 120 seconds in 100nS units

		status = KeWaitForSingleObject(&socket->iInitialised, Executive, KernelMode, false, &timeout);

		if (status == STATUS_SUCCESS)
		{
			return (socket);
		}
	}

	ExFreePoolWithTag(socket, '1ten');

	return (0);
}

NTSTATUS CSocketOhm::CreateComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
	UNREFERENCED_PARAMETER(aDeviceObject);

	if (aIrp->IoStatus.Status == STATUS_SUCCESS)
	{
		CSocketOhm* socket = (CSocketOhm*)aContext;

		// Save the socket object for the new socket
		socket->iSocket = (PWSK_SOCKET)(aIrp->IoStatus.Information);

		// Reuse the Irp
		IoReuseIrp(aIrp, STATUS_SUCCESS);

		// Set the completion routine for the IRP
		IoSetCompletionRoutine(aIrp, BindComplete, socket, TRUE, TRUE, TRUE);

		// Bind the socket

		SOCKADDR addr;

		CWinsock::Initialise(&addr);

		((PWSK_PROVIDER_DATAGRAM_DISPATCH)(socket->iSocket->Dispatch))->WskBind(socket->iSocket, &addr, 0, aIrp);
	}
	else
	{
		IoFreeIrp(aIrp);
	}

	// Always return STATUS_MORE_PROCESSING_REQUIRED to
	// terminate the completion processing of the IRP.
	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS CSocketOhm::BindComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
	UNREFERENCED_PARAMETER(aDeviceObject);

	if (aIrp->IoStatus.Status == STATUS_SUCCESS)
	{
		CSocketOhm* socket = (CSocketOhm*)aContext;

		KeSetEvent(&socket->iInitialised, 0, false);
	}

	// Free the IRP

	IoFreeIrp(aIrp);

	// Always return STATUS_MORE_PROCESSING_REQUIRED to
	// terminate the completion processing of the IRP.
	return STATUS_MORE_PROCESSING_REQUIRED;
}

void CSocketOhm::SetTtl(TUint aValue)
{
    UNREFERENCED_PARAMETER(aValue);

	PIRP irp;

	// Allocate an IRP

	irp = IoAllocateIrp(1, FALSE);

	// Check result

	if (irp == NULL)
	{
        return;
    }

	IoSetCompletionRoutine(irp,	SetTtlComplete, NULL, TRUE, TRUE, TRUE);

	SIZE_T returned;

	((PWSK_PROVIDER_DATAGRAM_DISPATCH)(iSocket->Dispatch))->Basic.WskControlSocket(iSocket, WskSetOption, IP_MULTICAST_TTL, IPPROTO_IP, 1, &aValue, 0, NULL, &returned, irp);
}

NTSTATUS CSocketOhm::SetTtlComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
    UNREFERENCED_PARAMETER(aDeviceObject);
    UNREFERENCED_PARAMETER(aContext);

	IoFreeIrp(aIrp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


void CSocketOhm::SetMulticastIf(TUint aValue)
{
    UNREFERENCED_PARAMETER(aValue);

	PIRP irp;

	// Allocate an IRP

	irp = IoAllocateIrp(1, FALSE);

	// Check result

	if (irp == NULL)
	{
        return;
    }

	IoSetCompletionRoutine(irp,	SetMulticastIfComplete, NULL, TRUE, TRUE, TRUE);

	SIZE_T returned;

	((PWSK_PROVIDER_DATAGRAM_DISPATCH)(iSocket->Dispatch))->Basic.WskControlSocket(iSocket, WskSetOption, IP_MULTICAST_IF, IPPROTO_IP, 4, &aValue, 0, NULL, &returned, irp);
}

NTSTATUS CSocketOhm::SetMulticastIfComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
    UNREFERENCED_PARAMETER(aDeviceObject);
    UNREFERENCED_PARAMETER(aContext);

	IoFreeIrp(aIrp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


void CSocketOhm::Send(WSK_BUF* aBuffer, SOCKADDR* aAddress, PIRP aIrp)
{
	((PWSK_PROVIDER_DATAGRAM_DISPATCH)(iSocket->Dispatch))->WskSendTo(iSocket, aBuffer, 0, aAddress, 0, NULL, aIrp);
}

void CSocketOhm::Close()
{
	PIRP irp;

	// Allocate an IRP

	irp = IoAllocateIrp(1, FALSE);

	// Check result

	if (irp == 0)
	{
        return;
    }

	KEVENT closed;

	KeInitializeEvent(&closed, NotificationEvent, false);

	// Set the completion routine for the IRP

	IoSetCompletionRoutine(irp,	CloseComplete, &closed, TRUE, TRUE, TRUE);

	((PWSK_PROVIDER_DATAGRAM_DISPATCH)(iSocket->Dispatch))->Basic.WskCloseSocket(iSocket, irp);

	KeWaitForSingleObject(&closed, Executive, KernelMode, false, NULL); // null = wait forever (no timeout)
}

NTSTATUS CSocketOhm::CloseComplete(PDEVICE_OBJECT aDeviceObject, PIRP aIrp, PVOID aContext)
{
    UNREFERENCED_PARAMETER(aDeviceObject);

	IoFreeIrp(aIrp);

	PKEVENT closed = (PKEVENT)aContext;

	KeSetEvent(closed, 0, false);

	// Always return STATUS_MORE_PROCESSING_REQUIRED to
	// terminate the completion processing of the IRP.

	return STATUS_MORE_PROCESSING_REQUIRED;
}

#endif

