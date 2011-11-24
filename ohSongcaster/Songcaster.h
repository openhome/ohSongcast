#ifndef HEADER_SONGCASTER
#define HEADER_SONGCASTER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>

#include "../Ohm.h"
#include "../OhmSender.h"
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
typedef void (STDCALL *ConfigurationChangedCallback)(void* aPtr, THandle aSongcaster);

DllExport THandle STDCALL SongcasterCreate(const char* aDomain, uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl, uint32_t aLatency, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, void* aImagePtr, uint32_t aImageBytes, const char* aMimeType);

DllExport uint32_t STDCALL SongcasterSubnet(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterChannel(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterTtl(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterLatency(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterMulticast(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterEnabled(THandle aSongcaster);
DllExport uint32_t STDCALL SongcasterPreset(THandle aSongcaster);

DllExport void STDCALL SongcasterSetSubnet(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetChannel(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetTtl(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetLatency(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetMulticast(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetEnabled(THandle aSongcaster, uint32_t aValue);
DllExport void STDCALL SongcasterSetPreset(THandle aSongcaster, uint32_t aValue);

DllExport void STDCALL SongcasterSetTrack(THandle aSongcaster, const char* aUri, const char* aMetadata, uint64_t aSamplesTotal, uint64_t aSampleStart);
DllExport void STDCALL SongcasterSetMetatext(THandle aSongcaster, const char* aValue);

DllExport void STDCALL SongcasterRefreshReceivers(THandle aSongcaster);

DllExport void STDCALL SongcasterDestroy(THandle aSongcaster);

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

class Songcaster;

class Receiver
{
	friend class Songcaster;

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
	friend class Songcaster;

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
};

class DllExportClass Songcaster : public IReceiverManager3Handler
{
public:
	static const TUint kMaxUdnBytes = 200;

public:
    Songcaster(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TUint aLatency, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, const Brx& aComputer, IOhmSenderDriver* aDriver, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, const Brx& aImage, const Brx& aMimeType);

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

	~Songcaster();

private:
	void SubnetListChanged();
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
	Mutex iMutex;
	TBool iClosing;
	TIpAddress iAdapter;
	std::vector<Subnet*> iSubnetList;
	OhmSender* iSender;
	IOhmSenderDriver* iDriver;
	DvDeviceStandard* iDevice;
	ReceiverManager3* iReceiverManager;
};

} // namespace Net
} // namespace OpenHome

#endif // HEADER_SONGCASTER

