#include "AudioUserClient.h"
#include "Songcast.h"
#include "AudioDeviceInterface.h"
#include <IOKit/IOLib.h>


// Declaration of private dispatcher class

class AudioUserClientDispatcher
{
public:
    static const IOExternalMethodDispatch iMethods[eNumDriverMethods];

    static IOReturn Open(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
    static IOReturn Close(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
    static IOReturn SetActive(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
    static IOReturn SetEndpoint(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
    static IOReturn SetTtl(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
};


// Definition of table for the dispatcher methods

const IOExternalMethodDispatch AudioUserClientDispatcher::iMethods[eNumDriverMethods] =
{
    // eOpen
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::Open, 0, 0, 0, 0
    },
    // eClose
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::Close, 0, 0, 0, 0
    },
    // eSetActive
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetActive, 1, 0, 0, 0
    },
    // eSetEndpoint
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetEndpoint, 2, 0, 0, 0
    },
    // eSetTtl
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetTtl, 1, 0, 0, 0
    }
};


// Definition of dispatch functions

IOReturn AudioUserClientDispatcher::Open(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->Open();
}

IOReturn AudioUserClientDispatcher::Close(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->Close();
}

IOReturn AudioUserClientDispatcher::SetActive(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetActive(aArgs->scalarInput[0]);
}

IOReturn AudioUserClientDispatcher::SetEndpoint(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetEndpoint(aArgs->scalarInput[0], aArgs->scalarInput[1]);
}

IOReturn AudioUserClientDispatcher::SetTtl(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetTtl(aArgs->scalarInput[0]);
}


// Implementation of audio user client class

OSDefineMetaClassAndStructors(AudioUserClient, IOUserClient);


IOReturn AudioUserClient::externalMethod(uint32_t aSelector, IOExternalMethodArguments* aArgs, IOExternalMethodDispatch* aDispatch, OSObject* aTarget, void* aReference)
{
    if (aSelector < eNumDriverMethods)
    {
        aDispatch = (IOExternalMethodDispatch*)&AudioUserClientDispatcher::iMethods[aSelector];
        if (!aTarget) {
            aTarget = this;
        }
    }

    return IOUserClient::externalMethod(aSelector, aArgs, aDispatch, aTarget, aReference);
}


bool AudioUserClient::start(IOService* aProvider)
{
    IOLog("Songcaster AudioUserClient[%p]::start(%p) ...\n", this, aProvider);

    // set the device that this user client is to talk to
    iDevice = OSDynamicCast(AudioDevice, aProvider);
    if (!iDevice) {
        IOLog("Songcaster AudioUserClient[%p]::start(%p) null device failure\n", this, aProvider);
        return false;
    }

    // make sure base class start is called after all other things that can fail
    if (!IOUserClient::start(aProvider)) {
        IOLog("Songcaster AudioUserClient[%p]::start(%p) base class start failed\n", this, aProvider);
        return false;
    }

    IOLog("Songcaster AudioUserClient[%p]::start(%p) ok\n", this, aProvider);
    return true;
}


void AudioUserClient::stop(IOService* aProvider)
{
    IOLog("Songcaster AudioUserClient[%p]::stop(%p)\n", this, aProvider);
    IOUserClient::stop(aProvider);
}


IOReturn AudioUserClient::clientClose()
{
    IOLog("Songcaster AudioUserClient[%p]::clientClose()\n", this);

    // terminate the user client - don't call the base class clientClose()
    terminate();

    return kIOReturnSuccess;
}


IOReturn AudioUserClient::clientDied()
{
    IOLog("Songcaster AudioUserClient[%p]::clientDied()\n", this);

    // the user space application has crashed - get the driver to stop sending audio
    if (DeviceOk() == kIOReturnSuccess)
    {
        iDevice->Socket().SetActive(false);
    }
    
    // base class calls clientClose()
    return IOUserClient::clientDied();
}


IOReturn AudioUserClient::DeviceOk()
{
    if (iDevice == 0 || isInactive()) {
        return kIOReturnNotAttached;
    }
    else if (!iDevice->isOpen(this)) {
        return kIOReturnNotOpen;
    }
    else {
        return kIOReturnSuccess;
    }
}


// eOpen

IOReturn AudioUserClient::Open()
{
    if (iDevice == 0 || isInactive()) {
        IOLog("Songcaster AudioUserClient[%p]::Open() not attached\n", this);
        return kIOReturnNotAttached;
    }
    else if (!iDevice->open(this)) {
        IOLog("Songcaster AudioUserClient[%p]::Open() exclusive access\n", this);
        return kIOReturnExclusiveAccess;
    }

    IOLog("Songcaster AudioUserClient[%p]::Open() ok\n", this);
    return kIOReturnSuccess;
}


// eClose

IOReturn AudioUserClient::Close()
{
    IOReturn ret = DeviceOk();
    if (ret != kIOReturnSuccess) {
        IOLog("Songcaster AudioUserClient[%p]::Close() returns %x\n", this, ret);
        return ret;
    }

    iDevice->close(this);
    IOLog("Songcaster AudioUserClient[%p]::Close() ok\n", this);
    return kIOReturnSuccess;
}


// eSetActive

IOReturn AudioUserClient::SetActive(uint64_t aActive)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->Socket().SetActive(aActive);
    }
    else {
        IOLog("Songcaster AudioUserClient[%p]::SetActive(%llu) returns %x\n", this, aActive, ret);
    }
    return ret;
}


// eSetEndpoint

IOReturn AudioUserClient::SetEndpoint(uint64_t aIpAddress, uint64_t aPort)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->Socket().Close();
        iDevice->Socket().Open(aIpAddress, aPort);
    }
    else {
        IOLog("Songcaster AudioUserClient[%p]::SetEndpoint(%llu, %llu) returns %x\n", this, aIpAddress, aPort, ret);
    }
    return ret;
}


// eSetTtl

IOReturn AudioUserClient::SetTtl(uint64_t aTtl)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->Socket().SetTtl(aTtl);
    }
    else {
        IOLog("Songcaster AudioUserClient[%p]::SetTtl(%llu) returns %x\n", this, aTtl, ret);
    }
    return ret;
}



