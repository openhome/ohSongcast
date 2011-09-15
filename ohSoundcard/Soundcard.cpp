#include "Soundcard.h"
#include "Icon.h"

#include <OpenHome/Private/Debug.h>

using namespace OpenHome;
using namespace OpenHome::Net;


// C interface

uint32_t STDCALL SoundcardSubnet(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetSubnet());
}

uint32_t STDCALL SoundcardChannel(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetChannel());
}

uint32_t STDCALL SoundcardTtl(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetTtl());
}

uint32_t STDCALL SoundcardMulticast(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetMulticast() ? 1 : 0);
}

uint32_t STDCALL SoundcardEnabled(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetEnabled() ? 1 : 0);
}

uint32_t STDCALL SoundcardPreset(THandle aSoundcard)
{
	return (((Soundcard*)aSoundcard)->GetPreset());
}

void STDCALL SoundcardSetSubnet(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetSubnet(aValue);
}

void STDCALL SoundcardSetChannel(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetChannel(aValue);
}

void STDCALL SoundcardSetTtl(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetTtl(aValue);
}

void STDCALL SoundcardSetMulticast(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetMulticast((aValue == 0) ? false : true);
}

void STDCALL SoundcardSetEnabled(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetEnabled((aValue == 0) ? false : true);
}

void STDCALL SoundcardSetPreset(THandle aSoundcard, uint32_t aValue)
{
	((Soundcard*)aSoundcard)->SetPreset(aValue);
}

void STDCALL SoundcardSetTrack(THandle aSoundcard, const char* aUri, const char* aMetadata, uint64_t aSamplesTotal, uint64_t aSampleStart)
{
	((Soundcard*)aSoundcard)->SetTrack(aUri, aMetadata, aSamplesTotal, aSampleStart);
}

void STDCALL SoundcardSetMetatext(THandle aSoundcard, const char* aValue)
{
	((Soundcard*)aSoundcard)->SetMetatext(aValue);
}

void STDCALL SoundcardRefreshReceivers(THandle aSoundcard)
{
	((Soundcard*)aSoundcard)->RefreshReceivers();
}

void STDCALL SoundcardDestroy(THandle aSoundcard)
{
	delete ((Soundcard*)aSoundcard);
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
		return (iAdapter ==	&aAdapter);
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
    
// Soundcard

Soundcard::Soundcard(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, const Brx& aComputer, IOhmSenderDriver* aDriver, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl)
	: iSubnet(aSubnet)
	, iChannel(aChannel)
	, iTtl(aTtl)
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

	Functor callback = MakeFunctor(*this, &Soundcard::SubnetListChanged);

	initParams->SetSubnetListChangedListener(callback);

	UpnpLibrary::Initialise(initParams);

	// Fixes bug in stack
	if (iSubnet == 0) {
		SubnetListChanged();
		iSubnet = iSubnetList[0]->Address();
		iAdapter = iSubnetList[0]->AdapterAddress();
	}
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

	Brn icon(icon_png, icon_png_len);

	iSender = new OhmSender(*iDevice, *iDriver, name, aChannel, iAdapter, aTtl, aMulticast, aEnabled, icon, Brn("image/png"), aPreset);
	
	iDevice->SetEnabled();

	iReceiverManager = new ReceiverManager3(*this, iSender->SenderUri(), iSender->SenderMetadata());
}

// Don't bother removing old subnets - they might come back anyway, and there is not exactly
// a huge traffic in added and removed network interfaces

void Soundcard::SubnetListChanged()
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

		// find adapter's subnet in current subnet list

		std::vector<Subnet*>::iterator it2 = iSubnetList.begin();

		while (it2 != iSubnetList.end()) {
			Subnet* subnet = *it2;

			if (subnet->Address() == adapter->Subnet()) {
				// adapter's subnet existed in the old subnet list, so check if the subnet is still using the same adapter

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
			return;
		}
	}
}

// return true if the current subnet was found in the list

TBool Soundcard::UpdateAdapter()
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

TIpAddress Soundcard::GetSubnet()
{
	iMutex.Wait();
	TIpAddress subnet = iSubnet;
	iMutex.Signal();
	return (subnet);
}

TUint Soundcard::GetChannel()
{
	iMutex.Wait();
	TUint channel = iChannel;
	iMutex.Signal();
	return (channel);
}

TUint Soundcard::GetTtl()
{
	iMutex.Wait();
	TUint ttl = iTtl;
	iMutex.Signal();
	return (ttl);
}

TBool Soundcard::GetMulticast()
{
	iMutex.Wait();
	TBool multicast = iMulticast;
	iMutex.Signal();
	return (multicast);
}

TBool Soundcard::GetEnabled()
{
	iMutex.Wait();
	TBool enabled = iEnabled;
	iMutex.Signal();
	return (enabled);
}

TUint Soundcard::GetPreset()
{
	iMutex.Wait();
	TUint preset = iPreset;
	iMutex.Signal();
	return (preset);
}

void Soundcard::SetSubnet(TIpAddress aValue)
{
	iMutex.Wait();

	if (iSubnet == aValue) {
		iMutex.Signal();
		return;
	}

	iSubnet = aValue;

	iMutex.Signal();

	UpnpLibrary::SetCurrentSubnet(iSubnet);

	ASSERT(UpdateAdapter());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetChannel(TUint aValue)
{
	iMutex.Wait();

	if (iChannel == aValue) {
		iMutex.Signal();
		return;
	}

	iChannel = aValue;

	iMutex.Signal();

	iSender->SetChannel(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetTtl(TUint aValue)
{
	iMutex.Wait();

	if (iTtl == aValue) {
		iMutex.Signal();
		return;
	}

	iTtl = aValue;

	iMutex.Signal();

	iSender->SetTtl(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetMulticast(TBool aValue)
{
	iMutex.Wait();

	if (iMulticast == aValue) {
		iMutex.Signal();
		return;
	}

	iMulticast = aValue;

	iMutex.Signal();

	iSender->SetMulticast(aValue);

	iReceiverManager->SetMetadata(iSender->SenderMetadata());

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetEnabled(TBool aValue)
{
	iMutex.Wait();

	if (iEnabled == aValue) {
		iMutex.Signal();
		return;
	}

	iEnabled = aValue;

	iMutex.Signal();

	iSender->SetEnabled(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetPreset(TUint aValue)
{
	iMutex.Wait();

	if (iPreset == aValue) {
		iMutex.Signal();
		return;
	}

	iPreset = aValue;

	iMutex.Signal();

	iSender->SetPreset(aValue);

	(*iConfigurationChangedCallback)(iConfigurationChangedPtr, this);
}

void Soundcard::SetTrack(const TChar* aUri, const TChar* aMetadata, TUint64 aSamplesTotal, TUint64 aSampleStart)
{
	iSender->SetTrack(Brn(aUri), Brn(aMetadata), aSamplesTotal, aSampleStart);
}

void Soundcard::SetMetatext(const TChar* aValue)
{
	iSender->SetMetatext(Brn(aValue));
}

void Soundcard::RefreshReceivers()
{
	iReceiverManager->Refresh();
}

Soundcard::~Soundcard()
{
    LOG(kMedia, "Soundcard::~Soundcard\n");

	delete (iReceiverManager);

    LOG(kMedia, "Soundcard::~Soundcard receiver manager destroyed\n");

	delete (iSender);

    LOG(kMedia, "Soundcard::~Soundcard sender destroyed\n");

	delete (iDevice);

    LOG(kMedia, "Soundcard::~Soundcard device destroyed\n");

	delete (iDriver);

    LOG(kMedia, "Soundcard::~Soundcard driver destroyed\n");

	iMutex.Wait();

	iClosing = true;

	iMutex.Signal();

    LOG(kMedia, "Soundcard::~Soundcard registered closing\n");

	std::vector<Subnet*>::iterator it = iSubnetList.begin();

	while (it != iSubnetList.end()) {
		Subnet* subnet = *it;
		(*iSubnetCallback)(iSubnetPtr, eRemoved, (THandle)subnet);
		delete (subnet);
		it++;
	}

    LOG(kMedia, "Soundcard::~Soundcard subnets destroyed\n");

	Net::UpnpLibrary::Close();

    LOG(kMedia, "Soundcard::~Soundcard library closed\n");
}

// IReceiverManager3Handler

void Soundcard::ReceiverAdded(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = new Receiver(aReceiver);
	aReceiver.SetUserData(receiver);
	(*iReceiverCallback)(iReceiverPtr, eAdded, (THandle)receiver);
}

void Soundcard::ReceiverChanged(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eChanged, (THandle)receiver);
}

void Soundcard::ReceiverRemoved(ReceiverManager3Receiver& aReceiver)
{
	Receiver* receiver = (Receiver*)(aReceiver.UserData());
	ASSERT(receiver);
	(*iReceiverCallback)(iReceiverPtr, eRemoved, (THandle)receiver);
	receiver->RemoveRef();
}
