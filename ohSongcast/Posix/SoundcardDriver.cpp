#include "../Songcast.h"

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>

#include "../../Ohm.h"
#include "../../OhmSender.h"

#include <sys/utsname.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


namespace OpenHome {
namespace Av {

class OhmSenderDriverPosix : public IOhmSenderDriver
{
public:
    OhmSenderDriverPosix();

private:
    // IOhmSenderDriver
    virtual void SetEnabled(TBool aValue);
	virtual void SetEndpoint(const Endpoint& aEndpoint, TIpAddress aAdapter);
    virtual void SetActive(TBool aValue);
    virtual void SetTtl(TUint aValue);
    virtual void SetLatency(TUint aValue);
    virtual void SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal);
	virtual void Resend(const Brx& aFrames);
};


} // namespace Av
} // namespace OpenHome


using namespace OpenHome;
using namespace OpenHome::Av;


OhmSenderDriverPosix::OhmSenderDriverPosix()
{
}

// IOhmSenderDriver
void OhmSenderDriverPosix::SetEnabled(TBool aValue)
{
    printf(aValue ? "OhmSenderDriverPosix: Enabled\n" : "OhmSenderDriverPosix: Disabled\n");
}

void OhmSenderDriverPosix::SetEndpoint(const Endpoint& aEndpoint, TIpAddress aAdapter)
{
    printf("OhmSenderDriverPosix: Endpoint %8x:%d, Adapter %8x\n", aEndpoint.Address(), aEndpoint.Port(), aAdapter);
}

void OhmSenderDriverPosix::SetActive(TBool aValue)
{
    printf(aValue ? "OhmSenderDriverPosix: Active\n" : "OhmSenderDriverPosix: Inactive\n");
}

void OhmSenderDriverPosix::SetTtl(TUint aValue)
{
    printf("OhmSenderDriverPosix: TTL %d\n", aValue);
}

void OhmSenderDriverPosix::SetLatency(TUint aValue)
{
    printf("OhmSenderDriverPosix: Latency %d\n", aValue);
}

void OhmSenderDriverPosix::SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal)
{
    printf("OhmSenderDriverPosix: TrackPosition %" PRIu64 " %" PRIu64 "\n", aSampleStart, aSamplesTotal);
}

void OhmSenderDriverPosix::Resend(const Brx& aFrames)
{
    printf("OhmSenderDriverPosix: Resend");

	ReaderBuffer buffer(aFrames);
	ReaderBinary reader(buffer);

	TUint frames = aFrames.Bytes() / 4;

	while (frames-- > 0) {
		printf(" %d", reader.ReadUintBe(4));
	}

	printf("\n");
}


// Soundcard - platform specific implementation of the C interface

THandle SongcastCreate(const char* aDomain, uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl,
                       uint32_t aLatency, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset,
                       ReceiverCallback aReceiverCallback, void* aReceiverPtr,
                       SubnetCallback aSubnetCallback, void* aSubnetPtr,
                       ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr,
                       MessageCallback aFatalErrorCallback, void* aFatalErrorPtr,
                       MessageCallback aLogOutputCallback, void* aLogOutputPtr,
                       const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl,
                       void* aImagePtr, uint32_t aImageBytes, const char* aMimeType)
{
    // get the computer name
    struct utsname name;
    if (uname(&name) < 0)
        return 0;

    Brn computer(name.nodename);

    // create the driver
    OhmSenderDriverPosix* driver = new OhmSenderDriverPosix();

    Songcast* songcast = new Songcast(aSubnet, aChannel, aTtl,
                                      aLatency, aMulticast, aEnabled, aPreset,
                                      aReceiverCallback, aReceiverPtr,
                                      aSubnetCallback, aSubnetPtr,
                                      aConfigurationChangedCallback, aConfigurationChangedPtr,
                                      aFatalErrorCallback, aFatalErrorPtr,
                                      aLogOutputCallback, aLogOutputPtr,
                                      computer, driver, aManufacturer, aManufacturerUrl, aModelUrl,
                                      Brn((TByte*)aImagePtr, aImageBytes), Brn(aMimeType));
    return songcast;
}




