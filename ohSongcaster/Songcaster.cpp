#include "Songcaster.h"
#include "Icon.h"

#include <OpenHome/Private/Debug.h>

using namespace OpenHome;
using namespace OpenHome::Net;


// C interface

uint32_t STDCALL SongcasterSubnet(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetSubnet());
}

uint32_t STDCALL SongcasterChannel(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetChannel());
}

uint32_t STDCALL SongcasterTtl(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetTtl());
}

uint32_t STDCALL SongcasterLatency(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetLatency());
}

uint32_t STDCALL SongcasterMulticast(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetMulticast() ? 1 : 0);
}

uint32_t STDCALL SongcasterEnabled(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetEnabled() ? 1 : 0);
}

uint32_t STDCALL SongcasterPreset(THandle aSongcaster)
{
	return (((Songcaster*)aSongcaster)->GetPreset());
}

void STDCALL SongcasterSetSubnet(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetSubnet(aValue);
}

void STDCALL SongcasterSetChannel(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetChannel(aValue);
}

void STDCALL SongcasterSetTtl(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetTtl(aValue);
}

void STDCALL SongcasterSetLatency(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetLatency(aValue);
}

void STDCALL SongcasterSetMulticast(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetMulticast((aValue == 0) ? false : true);
}

void STDCALL SongcasterSetEnabled(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetEnabled((aValue == 0) ? false : true);
}

void STDCALL SongcasterSetPreset(THandle aSongcaster, uint32_t aValue)
{
	((Songcaster*)aSongcaster)->SetPreset(aValue);
}

void STDCALL SongcasterSetTrack(THandle aSongcaster, const char* aUri, const char* aMetadata, uint64_t aSamplesTotal, uint64_t aSampleStart)
{
	((Songcaster*)aSongcaster)->SetTrack(aUri, aMetadata, aSamplesTotal, aSampleStart);
}

void STDCALL SongcasterSetMetatext(THandle aSongcaster, const char* aValue)
{
	((Songcaster*)aSongcaster)->SetMetatext(aValue);
}

void STDCALL SongcasterRefreshReceivers(THandle aSongcaster)
{
	((Songcaster*)aSongcaster)->RefreshReceivers();
}

void STDCALL SongcasterDestroy(THandle aSongcaster)
{
	delete ((Songcaster*)aSongcaster);
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
{
	AddRef();
}

Subnet::Subnet(TIpAddress aSubnet)
	: iAdapter(0)
	, iSubnet(aSubnet)
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
	RemoveRef();
	iAdapter = &aAdapter;
	AddRef();
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
	if (iAdapter != 0) {
		iAdapter->AddRef();
	}
}

void Subnet::RemoveRef()
{
	if (iAdapter != 0) {
		iAdapter->RemoveRef();
	}
}

Subnet::~Subnet()
{
	RemoveRef();
}
    
// Songcaster

Songcaster::Songcaster(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TUint aLatency, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, const Brx& aComputer, IOhmSenderDriver* aDriver, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl, const Brx& aImage, const Brx& aMimeType)
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
	, iMutex("SCRD")
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
    name.Append(" (");
    name.Append(aManufacturer);
    name.Append(" Songcaster)");
    // TODO: manufacturer will need to be parsed and spaces replaced with -
    udn.Replace(aManufacturer);
	udn.Append("-Songcaster-");
	udn.Append(aComputer);
	friendly.Replace(udn);
    description.Replace(aManufacturer);
    description.Append(" Songcaster");

	InitialisationParams* initParams = InitialisationParams::Create();

	Functor callback = MakeFunctor(*this, &Songcaster::SubnetListChanged);

	initParams->SetSubnetListChangedListener(callback);

	UpnpLibrary::Initialise(initParams);

	// Fixes bug in stack
	/* Removing this because I think stack is now fixed
	if (iSubnet == 0) {
		SubnetListChanged();
		iSubnet = iSubnetList[0]->Address();
		iAdapter = iSubnetList[0]->AdapterAddress();
	}
	*/
	/////////////////////

	UpnpLibrary::StartCombined(iSubnet);

	iDevice = new DvDeviceStandard(udn);
    
	iDevice->SetAttribute("Upnp.Domain", "av.openhome.org");
    iDevice->SetAttribute("Upnp.Type", "Songcaster");
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

	iSender = new OhmSender(*iDevice, *iDriver, name, iChannel, iAdapter, iTtl, iLatency, iMulticast, iEnabled, aImage, aMimeType, iPreset);
	
	iDevice->SetEnabled();

	iReceiverManager = new ReceiverManager3(*this, iSender->SenderUri(), iSender->SenderMetadata());
}

// Don't bother removing old subnets - they might come back anyway, and there is not exactly
// a huge traffic in added and removed network interfaces

void Songcaster::SubnetListChanged()
{
	iMutex.Wait();

	TBool closing = iClosing;

	iMutex.Signal();

	if (closing) {
		return;
	}

	// First, handle changes to the subnet list

	std::vector<NetworkAdapter*>*  subnetList = UpnpLibrary::CreateSubnetList();

	std::vector<NetworkAdapter*>::iterator it = subnetList->begin();

	while (it != subnetList->end()) {
		NetworkAdapter* adapter = *it;

		TBool found = false;

		// find new subnet in current subnet list

		std::vector<Subnet*>::iterator it2 = iSubnetList.begin();

		while (it2 != iSubnetList.end()) {
			Subnet* subnet = *it2;

			if (subnet->Address() == adapter->Subnet()) {
				// new subnet existed in the old subnet list, so check if the subnet is still using the same adapter

				if (!subnet->IsAttachedTo(*adapter))
				{
					subnet->Attach(*adapter);
					(*iSubnetCallback)(iSubnetPtr, eChanged, (THandle)subnet);
				}

				found = true;

				break;
			}

			it2++;
		}

		// if not found, this is a new subnet

		if (!found) {
			Subnet* subnet = new Subnet(*adapter);
			iSubnetList.push_back(subnet);
			(*iSubnetCallback)(iSubnetPtr, eAdded, (THandle)subnet);
		}

		it++;
	}

	UpnpLibrary::DestroySubnetList(subnetList);

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

TBool Songcaster::UpdateAdapter()
{
	std::vector<Subnet*>::iterator it = iSubnetList.begin();

	while (it != iSubnetList.end()) {
		Subnet* subnet = *it;
	
		if (iSubnet == subnet->Address())
		{
			TIpAddress adapter = subnet->AdapterAddress();

			if (iAdapter != adapter) {
				iAdapter = adapter;

				if (iSender != 0) {
					iSender->SetInterface(iAdapter);
				}
			}

			return (true);
		}

		it++;
	}

	return (false);
}

TIpAddress Songcaster::GetSubnet()
{
	iMutex.Wait();
	TIpAddress subnet = iSubnet;
	iMutex.Signal();
	return (subnet);
}

TUint Songcaster::GetChannel()
{
	iMutex.Wait();
	TUint channel = iChannel;
	iMutex.Signal();
	return (channel);
}

TUint Songcaster::GetTtl()
{
	iMutex.Wait();
	TUint ttl = iTtl;
	iMutex.Signal();
	return (ttl);
}

TUint Songcaster::GetLatency()
{
	iMutex.Wait();
	TUint latency = iLatency;
	iMutex.Signal();
	return (latency);
}

TBool Songcaster::GetMulticast()
{
	iMutex.Wait();
	TBool multicast = iMulticast;
	iMutex.Signal();
	return (multicast);
}

TBool Songcaster::GetEnabled()
{
	iMutex.Wait();
	TBool enabled = iEnabled;
	iMutex.Signal();
	return (enabled);
}

TUint Songcaster::GetPreset()
{
	iMutex.Wait();
	TUint preset = iPreset;
	iMutex.Signal();
	return (preset);
}

void Songcaster::SetSubnet(TIpAddress aValue)
{
	iMutex.Wait();

	if (iSubnet == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iSubnet = aValue;

	iMutex.Signal();

	UpnpLibrary::SetCurrentSubnet(iSubnet);

	ASSERT(UpdateAdapter());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetChannel(TUint aValue)
{
	iMutex.Wait();

	if (iChannel == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iChannel = aValue;

	iMutex.Signal();

	iSender->SetChannel(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetTtl(TUint aValue)
{
	iMutex.Wait();

	if (iTtl == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iTtl = aValue;

	iMutex.Signal();

	iSender->SetTtl(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetLatency(TUint aValue)
{
	iMutex.Wait();

	if (iLatency == aValue  || iClosing) {
		iMutex.Signal();
		return;
	}

	iLatency = aValue;

	iMutex.Signal();

	iSender->SetLatency(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetMulticast(TBool aValue)
{
	iMutex.Wait();

	if (iMulticast == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iMulticast = aValue;

	iMutex.Signal();

	iSender->SetMulticast(aValue);

	iReceiverManager->SetMetadata(iSender->SenderMetadata());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetEnabled(TBool aValue)
{
	iMutex.Wait();

	if (iEnabled == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iEnabled = aValue;

	iMutex.Signal();

	iSender->SetEnabled(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetPreset(TUint aValue)
{
	iMutex.Wait();

	if (iPreset == aValue || iClosing) {
		iMutex.Signal();
		return;
	}

	iPreset = aValue;

	iMutex.Signal();

	iSender->SetPreset(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Songcaster::SetTrack(const TChar* aUri, const TChar* aMetadata, TUint64 aSamplesTotal, TUint64 aSampleStart)
{
	iSender->SetTrack(Brn(aUri), Brn(aMetadata), aSamplesTotal, aSampleStart);
}

void Songcaster::SetMetatext(const TChar* aValue)
{
	iSender->SetMetatext(Brn(aValue));
}

void Songcaster::RefreshReceivers()
{
	iReceiverManager->Refresh();
}

Songcaster::~Songcaster()
{
    LOG(kMedia, "Songcaster::~Songcaster\n");

	iMutex.Wait();

	iClosing = true;

	iMutex.Signal();

    LOG(kMedia, "Songcaster::~Songcaster registered closing\n");

	delete (iReceiverManager);

    LOG(kMedia, "Songcaster::~Songcaster receiver manager destroyed\n");

	delete (iSender);

    LOG(kMedia, "Songcaster::~Songcaster sender destroyed\n");

	delete (iDevice);

    LOG(kMedia, "Songcaster::~Songcaster device destroyed\n");

	delete (iDriver);

    LOG(kMedia, "Songcaster::~Songcaster driver destroyed\n");

	std::vector<Subnet*>::iterator it = iSubnetList.begin();

	while (it != iSubnetList.end()) {
		Subnet* subnet = *it;
		(*iSubnetCallback)(iSubnetPtr, eRemoved, (THandle)subnet);
		delete (subnet);
		it++;
	}

    LOG(kMedia, "Songcaster::~Songcaster subnets destroyed\n");

	Net::UpnpLibrary::Close();

    LOG(kMedia, "Songcaster::~Songcaster library closed\n");
}

// IReceiverManager3Handler

void Songcaster::ReceiverAdded(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = new Receiver(aReceiver);
	aReceiver.SetUserData(receiver);
	(*iReceiverCallback)(iReceiverPtr, eAdded, (THandle)receiver);
}

void Songcaster::ReceiverChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eChanged, (THandle)receiver);
}

void Songcaster::ReceiverRemoved(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eRemoved, (THandle)receiver);
	receiver->RemoveRef();
}
