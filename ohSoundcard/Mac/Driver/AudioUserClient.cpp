#include "AudioUserClient.h"
#include <IOKit/IOLib.h>


OSDefineMetaClassAndStructors(AudioUserClient, IOUserClient);


const IOExternalMethodDispatch AudioUserClient::iMethods[eNumDriverMethods] =
{
    // eOpen
    {
        (IOExternalMethodAction)&AudioUserClient::DispatchOpen, 0, 0, 0, 0
    },
    // eClose
    {
        (IOExternalMethodAction)&AudioUserClient::DispatchClose, 0, 0, 0, 0
    },
    // eSetActive
    {
        (IOExternalMethodAction)&AudioUserClient::DispatchSetActive, 1, 0, 0, 0
    },
    // eSetEndpoint
    {
        (IOExternalMethodAction)&AudioUserClient::DispatchSetEndpoint, 2, 0, 0, 0
    },
    // eSetTtl
    {
        (IOExternalMethodAction)&AudioUserClient::DispatchSetTtl, 1, 0, 0, 0
    }
};


IOReturn AudioUserClient::externalMethod(uint32_t aSelector, IOExternalMethodArguments* aArgs, IOExternalMethodDispatch* aDispatch, OSObject* aTarget, void* aReference)
{
    if (aSelector < eNumDriverMethods)
    {
        aDispatch = (IOExternalMethodDispatch*)&iMethods[aSelector];
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
        iDevice->Socket().SetInactiveAndHalt();
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

IOReturn AudioUserClient::DispatchOpen(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->Open();
}

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

IOReturn AudioUserClient::DispatchClose(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->Close();
}

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

IOReturn AudioUserClient::DispatchSetActive(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetActive(aArgs->scalarInput[0]);
}

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

IOReturn AudioUserClient::DispatchSetEndpoint(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetEndpoint(aArgs->scalarInput[0], aArgs->scalarInput[1]);
}

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

IOReturn AudioUserClient::DispatchSetTtl(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetTtl(aArgs->scalarInput[0]);
}

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



