#include "Songcast.h"

#include "md5.c"

#include <OpenHome/Private/Debug.h>
#include <OpenHome/Private/Ascii.h>
#include "../Debug.h"

#include <algorithm>

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::Av;


// C interface

uint32_t STDCALL SongcastSubnet(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetSubnet());
}

uint32_t STDCALL SongcastChannel(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetChannel());
}

uint32_t STDCALL SongcastTtl(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetTtl());
}

uint32_t STDCALL SongcastLatency(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetLatency());
}

uint32_t STDCALL SongcastMulticast(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetMulticast() ? 1 : 0);
}

uint32_t STDCALL SongcastEnabled(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetEnabled() ? 1 : 0);
}

uint32_t STDCALL SongcastPreset(THandle aSongcast)
{
	return (((Songcast*)aSongcast)->GetPreset());
}

void STDCALL SongcastSetSubnet(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetSubnet(aValue);
}

void STDCALL SongcastSetChannel(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetChannel(aValue);
}

void STDCALL SongcastSetTtl(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetTtl(aValue);
}

void STDCALL SongcastSetLatency(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetLatency(aValue);
}

void STDCALL SongcastSetMulticast(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetMulticast((aValue == 0) ? false : true);
}

void STDCALL SongcastSetEnabled(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetEnabled((aValue == 0) ? false : true);
}

void STDCALL SongcastSetPreset(THandle aSongcast, uint32_t aValue)
{
	((Songcast*)aSongcast)->SetPreset(aValue);
}

void STDCALL SongcastSetTrack(THandle aSongcast, const char* aUri, const char* aMetadata, uint64_t aSamplesTotal, uint64_t aSampleStart)
{
	((Songcast*)aSongcast)->SetTrack(aUri, aMetadata, aSamplesTotal, aSampleStart);
}

void STDCALL SongcastSetMetatext(THandle aSongcast, const char* aValue)
{
	((Songcast*)aSongcast)->SetMetatext(aValue);
}

void STDCALL SongcastRefreshReceivers(THandle aSongcast)
{
	((Songcast*)aSongcast)->RefreshReceivers();
}

void STDCALL SongcastDestroy(THandle aSongcast)
{
	delete ((Songcast*)aSongcast);
}

const char* STDCALL ReceiverUdn(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Udn());
}

const char* STDCALL ReceiverRoom(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Room());
}

const char* STDCALL ReceiverGroup(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Group());
}

const char* STDCALL ReceiverName(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Name());
}

EReceiverStatus STDCALL ReceiverStatus(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Status());
}

uint32_t STDCALL ReceiverIpAddress(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->IpAddress());
}

bool STDCALL ReceiverHasVolumeControl(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->HasVolumeControl());
}

uint32_t STDCALL ReceiverVolume(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Volume());
}

bool STDCALL ReceiverMute(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->Mute());
}

uint32_t STDCALL ReceiverVolumeLimit(THandle aReceiver)
{
	return (((Receiver*)aReceiver)->VolumeLimit());
}

void STDCALL ReceiverSetVolume(THandle aReceiver, uint32_t aValue)
{
	((Receiver*)aReceiver)->SetVolume(aValue);
}

void STDCALL ReceiverVolumeInc(THandle aReceiver)
{
	((Receiver*)aReceiver)->VolumeInc();
}

void STDCALL ReceiverVolumeDec(THandle aReceiver)
{
	((Receiver*)aReceiver)->VolumeDec();
}

void STDCALL ReceiverSetMute(THandle aReceiver, bool aValue)
{
	((Receiver*)aReceiver)->SetMute(aValue);
}

void STDCALL ReceiverPlay(THandle aReceiver)
{
	((Receiver*)aReceiver)->Play();
}

void STDCALL ReceiverStop(THandle aReceiver)
{
	((Receiver*)aReceiver)->Stop();
}

void STDCALL ReceiverStandby(THandle aReceiver)
{
	((Receiver*)aReceiver)->Standby();
}

void STDCALL ReceiverAddRef(THandle aReceiver)
{
	((Receiver*)aReceiver)->AddRef();
}

void STDCALL ReceiverRemoveRef(THandle aReceiver)
{
	((Receiver*)aReceiver)->RemoveRef();
}

uint32_t STDCALL SubnetAddress(THandle aSubnet)
{
	return (((Subnet*)aSubnet)->Address());
}

const char* STDCALL SubnetAdapterName(THandle aSubnet)
{
	return (((Subnet*)aSubnet)->AdapterName());
}

void STDCALL SubnetAddRef(THandle aSubnet)
{
	((Subnet*)aSubnet)->AddRef();
}

void STDCALL SubnetRemoveRef(THandle aSubnet)
{
	((Subnet*)aSubnet)->RemoveRef();
}


// Receiver

Receiver::Receiver(ReceiverManager3Receiver& aReceiver)
	: iReceiver(aReceiver)
	, iUdn(iReceiver.Udn())
	, iRoom(iReceiver.Room())
	, iGroup(iReceiver.Group())
	, iName(iReceiver.Name())
	, iRefCount(1)
{
	iReceiver.AddRef();
}

const TChar* Receiver::Udn() const
{
	return (iUdn.CString());
}

const TChar* Receiver::Room() const
{
	return (iRoom.CString());
}

const TChar* Receiver::Group() const
{
	return (iGroup.CString());
}

const TChar* Receiver::Name() const
{
	return (iName.CString());
}

EReceiverStatus Receiver::Status() const
{
	return (EReceiverStatus)iReceiver.Status();
}

TIpAddress Receiver::IpAddress() const
{
    return iReceiver.IpAddress();
}

TBool Receiver::HasVolumeControl() const
{
	return iReceiver.HasVolumeControl();
}

TUint Receiver::Volume() const
{
	return iReceiver.Volume();
}

TBool Receiver::Mute() const
{
	return iReceiver.Mute();
}

TUint Receiver::VolumeLimit() const
{
	return iReceiver.VolumeLimit();
}


void Receiver::SetVolume(TUint aValue)
{
	iReceiver.SetVolume(aValue);
}

void Receiver::VolumeInc()
{
	iReceiver.VolumeInc();
}

void Receiver::VolumeDec()
{
	iReceiver.VolumeDec();
}

void Receiver::SetMute(TBool aValue)
{
	iReceiver.SetMute(aValue);
}

void Receiver::Play()
{
	iReceiver.Play();
}

void Receiver::Stop()
{
	iReceiver.Stop();
}

void Receiver::Standby()
{
	iReceiver.Standby();
}

void Receiver::AddRef()
{
	iRefCount++;
}

void Receiver::RemoveRef()
{
	if (--iRefCount == 0) {
		delete (this);
	}
}

Receiver::~Receiver()
{
	iReceiver.RemoveRef();
}

// Subnet

Subnet::Subnet(NetworkAdapter& aAdapter)
	: iAdapter(&aAdapter)
    , iSubnet(aAdapter.Subnet())
	, iRefCount(1)
{
	iAdapter->AddRef("Songcast");
}

Subnet::Subnet(TIpAddress aSubnet)
	: iAdapter(0)
	, iSubnet(aSubnet)
	, iRefCount(1)
{
}

TBool Subnet::IsAttachedTo(NetworkAdapter& aAdapter)
{
	if (iAdapter != 0) {
		return (iAdapter->Address() == aAdapter.Address());
	}
	return (false);
}

void Subnet::Attach(NetworkAdapter& aAdapter)
{
	if (iAdapter != 0) {
		iAdapter->RemoveRef("Songcast");
	}
	iAdapter = &aAdapter;
	iAdapter->AddRef("Songcast");
    ASSERT(iAdapter->Subnet() == iSubnet);
}

void Subnet::Detach()
{
	if (iAdapter != 0) {
		iAdapter->RemoveRef("Songcast");
	}
    iAdapter = 0;
}

TIpAddress Subnet::Address() const
{
	if (iAdapter != 0) {
		return (iAdapter->Subnet());
	}

	return (iSubnet);
}

TIpAddress Subnet::AdapterAddress() const
{
	if (iAdapter != 0) {
		return (iAdapter->Address());
	}

	return (0);
}

const TChar* Subnet::AdapterName() const
{
	if (iAdapter != 0) {
		return (iAdapter->Name());
	}

	return ("Network adapter not present");
}

void Subnet::AddRef()
{
	iRefCount++;
}

void Subnet::RemoveRef()
{
	if (--iRefCount == 0) {
		delete (this);
	}
}

Subnet::~Subnet()
{
	if (iAdapter != 0) {
		iAdapter->RemoveRef("Songcast");
	}
}
    
// Songcast

Songcast::Songcast(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TUint aLatency, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, MessageCallback aFatalErrorCallback, void* aFatalErrorPtr, MessageCallback aLogOutputCallback, void* aLogOutputPtr, const Brx& aComputer, IOhmSenderDriver* aDriver, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, const Brx& aImage, const Brx& aMimeType)
	: iSubnet(aSubnet)
	, iChannel(aChannel)
	, iTtl(aTtl)
	, iLatency(aLatency)
	, iMulticast(aMulticast)
	, iEnabled(aEnabled)
	, iPreset(aPreset)
	, iReceiverCallback(aReceiverCallback)
	, iReceiverPtr(aReceiverPtr)
	, iSubnetCallback(aSubnetCallback)
	, iSubnetPtr(aSubnetPtr)
	, iConfigurationChangedCallback(aConfigurationChangedCallback)
	, iConfigurationChangedPtr(aConfigurationChangedPtr)
    , iFatalErrorCallback(aFatalErrorCallback)
    , iFatalErrorPtr(aFatalErrorPtr)
	, iLogOutputCallback(aLogOutputCallback)
	, iLogOutputPtr(aLogOutputPtr)
	, iClosing(false)
	, iAdapter(0)
	, iSender(0)
    , iDriver(aDriver)
{
	//Debug::SetLevel(Debug::kMedia);

	Bws<kMaxUdnBytes> udn;
    Bws<kMaxUdnBytes> name;
	Bws<kMaxUdnBytes + 1> friendly;
    Bws<kMaxUdnBytes + 1> description;

    name.Replace(aComputer);

    // TODO: manufacturer will need to be parsed and spaces replaced with -

    friendly.Replace(aManufacturer);
	friendly.Append("-Songcast-");
	friendly.Append(aComputer);

	MD5_CTX ctx;

	TByte md5[16];

	MD5_Init(&ctx);
	MD5_Update(&ctx, (unsigned char*) friendly.Ptr(), friendly.Bytes());
	MD5_Final(md5, &ctx);

	for (TUint i = 0; i < 16; i++) {
		Ascii::AppendHex(udn, md5[i]);
	}

    description.Replace(aManufacturer);
    description.Append(" Songcast");

	InitialisationParams* initParams = InitialisationParams::Create();

    if (iFatalErrorCallback) {
        // only set the fatal error handler if it has been specified - use the default ohnet handler otherwise
        FunctorMsg fatal = MakeFunctorMsg(*this, &Songcast::FatalErrorHandler);
        initParams->SetFatalErrorHandler(fatal);
    }

    if (iLogOutputCallback) {
		// only set the log output handler if it has been specified - use the default ohnet handler otherwise
		FunctorMsg logOutput = MakeFunctorMsg(*this, &Songcast::LogOutputHandler);
		initParams->SetLogOutput(logOutput);
	}

	Functor callback = MakeFunctor(*this, &Songcast::SubnetListChanged);

	Debug::SetLevel(Debug::kTrace);

	initParams->SetSubnetListChangedListener(callback);

    // create the OhNet library
    iLibrary = new Library(initParams);

    // it is now ok to create the mutex
    iMutex = new Mutex("SCRD");

	CpStack* cpStack;
    DvStack* dvStack;
    iLibrary->StartCombined(iSubnet, cpStack, dvStack);

	iDevice = new DvDeviceStandard(*dvStack, udn);
    
	iDevice->SetAttribute("Upnp.Domain", "av.openhome.org");
    iDevice->SetAttribute("Upnp.Type", "Songcast");
    iDevice->SetAttribute("Upnp.Version", "1");
    iDevice->SetAttribute("Upnp.FriendlyName", (TChar*)friendly.PtrZ());
    iDevice->SetAttribute("Upnp.Manufacturer", (TChar*)description.PtrZ());
    iDevice->SetAttribute("Upnp.ManufacturerUrl", (TChar*)aManufacturerUrl);
    iDevice->SetAttribute("Upnp.ModelDescription", (TChar*)description.PtrZ());
    iDevice->SetAttribute("Upnp.ModelName", (TChar*)description.PtrZ());
    iDevice->SetAttribute("Upnp.ModelNumber", "1");
    iDevice->SetAttribute("Upnp.ModelUrl", (TChar*)aModelUrl);
    iDevice->SetAttribute("Upnp.SerialNumber", "");
    iDevice->SetAttribute("Upnp.Upc", "");

	SubnetListChanged();

	Environment& env = iLibrary->Env();
    iSender = new OhmSender(env, *iDevice, *iDriver, name, iChannel, iAdapter, iTtl, iLatency, iMulticast, iEnabled, aImage, aMimeType, iPreset);
	//iNetworkMonitor = new NetworkMonitor(env, *iDevice, name);
	iDevice->SetEnabled();
	iReceiverManager = new ReceiverManager3(*cpStack, *this, iSender->SenderUri(), iSender->SenderMetadata());
}


void Songcast::FatalErrorHandler(const char* aMessage)
{
    (*iFatalErrorCallback)(iFatalErrorPtr, aMessage);
}

void Songcast::LogOutputHandler(const char* aMessage)
{
	(*iLogOutputCallback)(iLogOutputPtr, aMessage);
}


/*
#include <Objbase.h>

struct MySEHExceptionStruct
{
  char* m_lpszMessage1;
  char m_szMessage2[256];
};

void Songcast::FatalErrorHandler(const char* aMessage)
{
 // First allocate space for a MySEHExceptionStruct instance.
  MySEHExceptionStruct* pMySEHExceptionStruct=(MySEHExceptionStruct*)::CoTaskMemAlloc(sizeof(MySEHExceptionStruct));
  // Zero out all bytes inside pMySEHExceptionStruct.
  memset (pMySEHExceptionStruct, 0, sizeof(MySEHExceptionStruct));

  // Assign value to the m_lpszMessage member.
  const char* lpszMessage1 = "SEH Exception Message 1.";
  const char* lpszMessage2 = "SEH Exception Message 2.";

  pMySEHExceptionStruct -> m_lpszMessage1 = (char*)::CoTaskMemAlloc(strlen(lpszMessage1) + 1);
  strcpy(pMySEHExceptionStruct -> m_lpszMessage1, lpszMessage1);
  strcpy(pMySEHExceptionStruct -> m_szMessage2, lpszMessage2);

  // Raise the SEH exception, passing along a ptr to the MySEHExceptionStruct
  // structure. Note that the onus is on the recipient of the exception to free
  // the memory of pMySEHExceptionStruct as well as its contents.
  RaiseException(100, 0, 1, (const ULONG_PTR*)(pMySEHExceptionStruct));
}
*/

// Simple predicate class for finding NetworkAdapter and Subnet objects with a given
// subnet TIpAddress in a list - simplfies the code in the SubnetListChanged method

class SubnetFinder
{
public:
    SubnetFinder(TIpAddress aSubnet) : iSubnet(aSubnet) {}

    bool operator()(NetworkAdapter* aAdapter) const {
        return (aAdapter->Subnet() == iSubnet);
    }

    bool operator()(Subnet* aSubnet) const {
        return (aSubnet->Address() == iSubnet);
    }

private:
    TIpAddress iSubnet;
};

// Don't bother removing old subnets - they might come back anyway, and there is not exactly
// a huge traffic in added and removed network interfaces

void Songcast::SubnetListChanged()
{
	iMutex->Wait();

	TBool closing = iClosing;

	iMutex->Signal();

	if (closing) {
		return;
	}

    // get the new subnet list from ohnet
    std::vector<NetworkAdapter*>* subnetList = iLibrary->CreateSubnetList();

    std::vector<NetworkAdapter*>::iterator newSubnetListIt;
    std::vector<Subnet*>::iterator oldSubnetListIt;

    // look for new subnets that already exist in or need to be added to the current list 
    for (newSubnetListIt = subnetList->begin() ; newSubnetListIt != subnetList->end() ; newSubnetListIt++)
    {
        // iterator is the adapter to use for this new subnet
        NetworkAdapter* adapter = *newSubnetListIt;

        // look for this new subnet in the current subnet list
        oldSubnetListIt = std::find_if(iSubnetList.begin(), iSubnetList.end(), SubnetFinder(adapter->Subnet()));

        if (oldSubnetListIt != iSubnetList.end())
        {
            // the new subnet already exists in the current subnet list
            Subnet* subnet = *oldSubnetListIt;

            if (!subnet->IsAttachedTo(*adapter))
            {
                // the corresponding subnet in the current list is attached to a different adapter, so attach this new one
                subnet->Attach(*adapter);
                (*iSubnetCallback)(iSubnetPtr, eChanged, (THandle)subnet);
            }
        }
        else
        {
            // the new subnet is not in the current list - add it
            Subnet* subnet = new Subnet(*adapter);
            iSubnetList.push_back(subnet);
            (*iSubnetCallback)(iSubnetPtr, eAdded, (THandle)subnet);
        }
    }

    // now look for subnets in the current list that are absent from the new list
    for (oldSubnetListIt = iSubnetList.begin() ; oldSubnetListIt != iSubnetList.end() ; oldSubnetListIt++)
    {
        Subnet* subnet = *oldSubnetListIt;

        // look for this subnet in the new list
        if (std::find_if(subnetList->begin(), subnetList->end(), SubnetFinder(subnet->Address())) == subnetList->end())
        {
            // this subnet is not in the new list so detach it from its current adapter
            subnet->Detach();
            (*iSubnetCallback)(iSubnetPtr, eChanged, (THandle)subnet);
        }
    }

	Library::DestroySubnetList(subnetList);

	// Now manage our current subnet and adapter

	if (!UpdateAdapter()) {

		// Not found - make a dummy subnet entry to represent our current subnet (unless 0)

		if (iSubnet != 0)
		{
			Subnet* subnet = new Subnet(iSubnet);
			iSubnetList.push_back(subnet);
			(*iSubnetCallback)(iSubnetPtr, eAdded, (THandle)subnet);
		}
	}
}

// return true if the current subnet was found in the list

TBool Songcast::UpdateAdapter()
{
	std::vector<Subnet*>::iterator it = std::find_if(iSubnetList.begin(), iSubnetList.end(), SubnetFinder(iSubnet));

    if (it != iSubnetList.end())
    {
        // the subnet exists in the subnet list - update the adapter interface if necessary
        Subnet* subnet = *it;
        TIpAddress adapter = subnet->AdapterAddress();

        if (iAdapter != adapter) {
            iAdapter = adapter;

            if (iSender != 0) {
                iSender->SetInterface(iAdapter);
            }
        }
    }

    return (it != iSubnetList.end());
}

TIpAddress Songcast::GetSubnet()
{
	iMutex->Wait();
	TIpAddress subnet = iSubnet;
	iMutex->Signal();
	return (subnet);
}

TUint Songcast::GetChannel()
{
	iMutex->Wait();
	TUint channel = iChannel;
	iMutex->Signal();
	return (channel);
}

TUint Songcast::GetTtl()
{
	iMutex->Wait();
	TUint ttl = iTtl;
	iMutex->Signal();
	return (ttl);
}

TUint Songcast::GetLatency()
{
	iMutex->Wait();
	TUint latency = iLatency;
	iMutex->Signal();
	return (latency);
}

TBool Songcast::GetMulticast()
{
	iMutex->Wait();
	TBool multicast = iMulticast;
	iMutex->Signal();
	return (multicast);
}

TBool Songcast::GetEnabled()
{
	iMutex->Wait();
	TBool enabled = iEnabled;
	iMutex->Signal();
	return (enabled);
}

TUint Songcast::GetPreset()
{
	iMutex->Wait();
	TUint preset = iPreset;
	iMutex->Signal();
	return (preset);
}

void Songcast::SetSubnet(TIpAddress aValue)
{
	iMutex->Wait();

	if (iSubnet == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iSubnet = aValue;

	iMutex->Signal();

	iLibrary->SetCurrentSubnet(iSubnet);

	ASSERT(UpdateAdapter());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetChannel(TUint aValue)
{
	iMutex->Wait();

	if (iChannel == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iChannel = aValue;

	iMutex->Signal();

	iSender->SetChannel(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetTtl(TUint aValue)
{
	iMutex->Wait();

	if (iTtl == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iTtl = aValue;

	iMutex->Signal();

	iSender->SetTtl(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetLatency(TUint aValue)
{
	iMutex->Wait();

	if (iLatency == aValue  || iClosing) {
	    iMutex->Signal();
		return;
	}

	iLatency = aValue;

	iMutex->Signal();

	iSender->SetLatency(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetMulticast(TBool aValue)
{
	iMutex->Wait();

	if (iMulticast == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iMulticast = aValue;

	iMutex->Signal();

	iSender->SetMulticast(aValue);

	iReceiverManager->SetMetadata(iSender->SenderMetadata());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetEnabled(TBool aValue)
{
	iMutex->Wait();

	if (iEnabled == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iEnabled = aValue;

	iMutex->Signal();

	iSender->SetEnabled(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetPreset(TUint aValue)
{
	iMutex->Wait();

	if (iPreset == aValue || iClosing) {
	    iMutex->Signal();
		return;
	}

	iPreset = aValue;

	iMutex->Signal();

	iSender->SetPreset(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcast::SetTrack(const TChar* aUri, const TChar* aMetadata, TUint64 aSamplesTotal, TUint64 aSampleStart)
{
	iSender->SetTrack(Brn(aUri), Brn(aMetadata), aSamplesTotal, aSampleStart);
}

void Songcast::SetMetatext(const TChar* aValue)
{
	iSender->SetMetatext(Brn(aValue));
}

void Songcast::RefreshReceivers()
{
	iReceiverManager->Refresh();
}

Songcast::~Songcast()
{
    LOG(kMedia, "Songcast::~Songcast\n");

	iMutex->Wait();
	iClosing = true;
	iMutex->Signal();

    LOG(kMedia, "Songcast::~Songcast registered closing\n");

	delete (iReceiverManager);
    LOG(kMedia, "Songcast::~Songcast receiver manager destroyed\n");

	//delete (iNetworkMonitor);
	//LOG(kMedia, "Songcast::~Songcast network monitor destroyed\n");

	delete (iSender);
    LOG(kMedia, "Songcast::~Songcast sender destroyed\n");

	delete (iDevice);
    LOG(kMedia, "Songcast::~Songcast device destroyed\n");

	delete (iDriver);
    LOG(kMedia, "Songcast::~Songcast driver destroyed\n");

	std::vector<Subnet*>::iterator it = iSubnetList.begin();
	while (it != iSubnetList.end()) {
		Subnet* subnet = *it;
		(*iSubnetCallback)(iSubnetPtr, eRemoved, (THandle)subnet);
		subnet->RemoveRef();
		it++;
	}
    LOG(kMedia, "Songcast::~Songcast subnets destroyed\n");

    delete iMutex;
    delete iLibrary;

    LOG(kMedia, "Songcast::~Songcast library closed\n");
}

// IReceiverManager3Handler

void Songcast::ReceiverAdded(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = new Receiver(aReceiver);
	aReceiver.SetUserData(receiver);
	(*iReceiverCallback)(iReceiverPtr, eAdded, (THandle)receiver);
}

void Songcast::ReceiverChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eChanged, (THandle)receiver);
}

void Songcast::ReceiverRemoved(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eRemoved, (THandle)receiver);
	receiver->RemoveRef();
}

void Songcast::ReceiverVolumeControlChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eVolumeControlChanged, (THandle)receiver);
}

void Songcast::ReceiverVolumeChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eVolumeChanged, (THandle)receiver);
}

void Songcast::ReceiverMuteChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eMuteChanged, (THandle)receiver);
}

void Songcast::ReceiverVolumeLimitChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eVolumeLimitChanged, (THandle)receiver);
}
