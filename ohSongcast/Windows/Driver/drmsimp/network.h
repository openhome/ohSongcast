/********************************************************************************
**    Copyright (c) 1998-2000 Microsoft Corporation. All Rights Reserved.
**
**       Portions Copyright (c) 1998-1999 Intel Corporation
**
********************************************************************************/

#ifndef _NETWORK_H_
#define _NETWORK_H_

#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4214) // bit field types other than int

#include <OpenHome/OhNetTypes.h>

#include <wdm.h>
#include <wsk.h>

#pragma warning(pop)

#if (NTDDI_VERSION >= NTDDI_VISTA)

/*****************************************************************************
 * Constants
 *****************************************************************************
 */

/*****************************************************************************
 * Classes
 *****************************************************************************
 */

class CSocketOhm;

class CWinsock
{
	friend class CSocketOhm;

public:
	static CWinsock* Create();
	static void Initialise(PSOCKADDR aSocket);
	static void Initialise(PSOCKADDR aSocket, ULONG aAddress, ULONG aPort);
	void Close();

private:
	static KSTART_ROUTINE Init;

private:
	KEVENT iInitialised;
	WSK_CLIENT_DISPATCH iAppDispatch;
	WSK_REGISTRATION iRegistration;
	WSK_PROVIDER_NPI iProviderNpi;
};

class CSocketOhm
{
	static const ULONG kMediaLatencyMs = 100;

public:
	static CSocketOhm* Create(CWinsock& aWsk);
	void SetTtl(TUint aValue);
	void SetMulticastIf(TUint aValue);
	void Send(WSK_BUF* aBuffer, SOCKADDR* aAddress, PIRP aIrp);
	void Close();

private:
	static IO_COMPLETION_ROUTINE CreateComplete;
	static IO_COMPLETION_ROUTINE BindComplete;
	static IO_COMPLETION_ROUTINE CloseComplete;
	static IO_COMPLETION_ROUTINE SetTtlComplete;
	static IO_COMPLETION_ROUTINE SetMulticastIfComplete;

private:
	CWinsock* iWsk;
	KEVENT iInitialised;
	PWSK_SOCKET iSocket;
};

#endif          // (NTDDI_VERSION >= NTDDI_VISTA)

#endif          // _NETWORK_H_


