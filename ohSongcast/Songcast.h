#ifndef HEADER_SONGCAST
#define HEADER_SONGCAST

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>

#include "../Ohm.h"
#include "../OhmSender.h"
#include "../../ohNetmon/NetworkMonitor.h"
#include "ReceiverManager3.h"

////////////////////////////////////////////
// Exported C interface

#ifdef __cplusplus
extern "C" {
#endif

enum ECallbackType {
	eAdded,
	eChanged,
	eRemoved
};

enum EReceiverStatus {
    eDisconnected,
    eConnecting,
    eConnected
};

/**
 * Callback which runs to notify a change in the networked receivers
 * @ingroup Callbacks
 *
 * @param[in] aPtr      Client-specified data
 * @param[in] aType     Type of change indicated
 * @param[in] aReceiver Receiver handle
 */
typedef void (STDCALL *ReceiverCallback)(void* aPtr, ECallbackType aType, THandle aReceiver);
typedef void (STDCALL *SubnetCallback)(void* aPtr, ECallbackType aType, THandle aSubnet);
typedef void (STDCALL *ConfigurationChangedCallback)(void* aPtr, THandle aSongcast);
typedef void (STDCALL *FatalErrorCallback)(void* aPtr, const char* aMessage);

DllExport THandle STDCALL SongcastCreate(const char* aDomain, uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl, uint32_t aLatency, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, FatalErrorCallback aFatalErrorCallback, void* aFatalErrorPtr, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, void* aImagePtr, uint32_t aImageBytes, const char* aMimeType);

DllExport uint32_t STDCALL SongcastSubnet(THandle aSongcast);
DllExport uint32_t STDCALL SongcastChannel(THandle aSongcast);
DllExport uint32_t STDCALL SongcastTtl(THandle aSongcast);
DllExport uint32_t STDCALL SongcastLatency(THandle aSongcast);
DllExport uint32_t STDCALL SongcastMulticast(THandle aSongcast);
DllExport uint32_t STDCALL SongcastEnabled(THandle aSongcast);
DllExport uint32_t STDCALL SongcastPreset(THandle aSongcast);

DllExport void STDCALL SongcastSetSubnet(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetChannel(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetTtl(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetLatency(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetMulticast(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetEnabled(THandle aSongcast, uint32_t aValue);
DllExport void STDCALL SongcastSetPreset(THandle aSongcast, uint32_t aValue);

DllExport void STDCALL SongcastSetTrack(THandle aSongcast, const char* aUri, const char* aMetadata, uint64_t aSamplesTotal, uint64_t aSampleStart);
DllExport void STDCALL SongcastSetMetatext(THandle aSongcast, const char* aValue);

DllExport void STDCALL SongcastRefreshReceivers(THandle aSongcast);

DllExport void STDCALL SongcastDestroy(THandle aSongcast);

DllExport const char* STDCALL ReceiverUdn(THandle aReceiver);
DllExport const char* STDCALL ReceiverRoom(THandle aReceiver);
DllExport const char* STDCALL ReceiverGroup(THandle aReceiver);
DllExport const char* STDCALL ReceiverName(THandle aReceiver);
DllExport EReceiverStatus STDCALL ReceiverStatus(THandle aReceiver);
DllExport void STDCALL ReceiverPlay(THandle aReceiver);
DllExport void STDCALL ReceiverStop(THandle aReceiver);
DllExport void STDCALL ReceiverStandby(THandle aReceiver);
DllExport void STDCALL ReceiverAddRef(THandle aReceiver);
DllExport void STDCALL ReceiverRemoveRef(THandle aReceiver);

DllExport uint32_t STDCALL SubnetAddress(THandle aSubnet);
DllExport const char* STDCALL SubnetAdapterName(THandle aSubnet);
DllExport void STDCALL SubnetAddRef(THandle aReceiver);
DllExport void STDCALL SubnetRemoveRef(THandle aReceiver);

#ifdef __cplusplus
} // extern "C"
#endif

////////////////////////////////////////////

namespace OpenHome {
namespace Net {

class Songcast;

class Receiver
{
	friend class Songcast;

public:
	const TChar* Udn() const;
	const TChar* Room() const;
	const TChar* Group() const;
	const TChar* Name() const;
	EReceiverStatus Status() const;

	void Play();
	void Stop();
	void Standby();

	void AddRef();
    void RemoveRef();
    
private:
	Receiver(ReceiverManager3Receiver& aReceiver);
	~Receiver();

private:
	ReceiverManager3Receiver& iReceiver;
	Brhz iUdn;
	Brhz iRoom;
	Brhz iGroup;
	Brhz iName;
    TUint iRefCount;
};

class Subnet : public INonCopyable
{
	friend class Songcast;

public:
	TIpAddress Address() const;
	const TChar* AdapterName() const;
	TIpAddress AdapterAddress() const;

	void AddRef();
    void RemoveRef();
    
private:
	Subnet(NetworkAdapter& aAdapter);
	Subnet(TIpAddress aSubnet);
	void Attach(NetworkAdapter& aAdapter);
	void Detach();
	TBool IsAttachedTo(NetworkAdapter& aAdapter);
	~Subnet();

private:
	NetworkAdapter* iAdapter;
	TIpAddress iSubnet;
	TUint iRefCount;
};

class DllExportClass Songcast : public IReceiverManager3Handler
{
public:
	static const TUint kMaxUdnBytes = 200;

public:
    Songcast(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TUint aLatency, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, FatalErrorCallback aFatalErrorCallback, void* aFatalErrorPtr, const Brx& aComputer, IOhmSenderDriver* aDriver, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, const Brx& aImage, const Brx& aMimeType);

	TIpAddress GetSubnet();
	TUint GetChannel();
	TUint GetTtl();
	TUint GetLatency();
	TBool GetMulticast();
	TBool GetEnabled();
	TUint GetPreset();

	void SetSubnet(TIpAddress aValue);
	void SetChannel(TUint aValue);
    void SetTtl(TUint aValue);
    void SetLatency(TUint aValue);
    void SetMulticast(TBool aValue);
	void SetEnabled(TBool aValue);
	void SetPreset(TUint aValue);
    void SetTrack(const TChar* aUri, const TChar* aMetadata, TUint64 aSamplesTotal, TUint64 aSampleStart);
	void SetMetatext(const TChar* aValue);
	void RefreshReceivers();

	~Songcast();

private:
	void SubnetListChanged();
	void FatalErrorHandler(const char* aMessage);
	TBool UpdateAdapter();

	// IReceiverManager3Handler
	virtual void ReceiverAdded(ReceiverManager3Receiver& aReceiver);
	virtual void ReceiverChanged(ReceiverManager3Receiver& aReceiver);
	virtual void ReceiverRemoved(ReceiverManager3Receiver& aReceiver);

private:
	TIpAddress iSubnet;
	TUint iChannel;
	TUint iTtl;
	TUint iLatency;
	TBool iMulticast;
	TBool iEnabled;
	TUint iPreset;
	ReceiverCallback iReceiverCallback;
	void* iReceiverPtr;
	SubnetCallback iSubnetCallback;
	void* iSubnetPtr;
	ConfigurationChangedCallback iConfigurationChangedCallback;
	void* iConfigurationChangedPtr;
    FatalErrorCallback iFatalErrorCallback;
    void* iFatalErrorPtr;
	Mutex iMutex;
	TBool iClosing;
	TIpAddress iAdapter;
	std::vector<Subnet*> iSubnetList;
	OhmSender* iSender;
	NetworkMonitor* iNetworkMonitor;
	IOhmSenderDriver* iDriver;
	DvDeviceStandard* iDevice;
	ReceiverManager3* iReceiverManager;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_SONGCAST

