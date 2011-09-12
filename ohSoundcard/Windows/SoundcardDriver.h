#ifndef HEADER_SOUNDCARDDRIVER
#define HEADER_SOUNDCARDDRIVER

#define INITGUID  // For PKEY_AudioEndpoint_GUID

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>
#include <Windows.h>

#include "../../Ohm.h"
#include "../../OhmSender.h"
#include "../Soundcard.h"

namespace OpenHome {
namespace Net {

class OhmSenderDriverWindows : public IOhmSenderDriver
{
public:
	OhmSenderDriverWindows(const char* aHardwareId);

private:    
	TBool FindDriver(const char* aHardwareId);
	TBool InstallDriver();
	TBool FindEndpoint();
	void SetDefaultAudioPlaybackDevice();
	void SetEndpointEnabled(TBool aValue);

	// IOhmSenderDriver
	virtual void SetEnabled(TBool aValue);
	virtual void SetEndpoint(const Endpoint& aEndpoint);
	virtual void SetActive(TBool aValue);
	virtual void SetTtl(TUint aValue);
	virtual void SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal);

private:
	HANDLE iHandle;
	WCHAR iEndpointId[100];

};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_SOUNDCARDDRIVER

