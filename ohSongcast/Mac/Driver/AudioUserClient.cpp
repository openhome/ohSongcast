#include "AudioUserClient.h"
#include "Songcast.h"
#include "AudioDeviceInterface.h"
#include <IOKit/IOLib.h>
#include <IOKit/IOCommandGate.h>


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
    static IOReturn SetLatencyMs(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
    static IOReturn Resend(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs);
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
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetEndpoint, 3, 0, 0, 0
    },
    // eSetTtl
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetTtl, 1, 0, 0, 0
    },
    // eSetLatencyMs
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::SetLatencyMs, 1, 0, 0, 0
    },
    // eResend
    {
        (IOExternalMethodAction)&AudioUserClientDispatcher::Resend, 1, ResendMaxBytes, 0, 0
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
    return aTarget->SetEndpoint(aArgs->scalarInput[0], aArgs->scalarInput[1], aArgs->scalarInput[2]);
}

IOReturn AudioUserClientDispatcher::SetTtl(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetTtl(aArgs->scalarInput[0]);
}

IOReturn AudioUserClientDispatcher::SetLatencyMs(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->SetLatencyMs(aArgs->scalarInput[0]);
}

IOReturn AudioUserClientDispatcher::Resend(AudioUserClient* aTarget, void* aReference, IOExternalMethodArguments* aArgs)
{
    return aTarget->Resend(aArgs->scalarInput[0], (const uint32_t*)aArgs->structureInput);
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
    IOLog("Songcast AudioUserClient[%p]::start(%p) ...\n", this, aProvider);

    iDevice = 0;
    iWorkLoop = 0;
    iCommandGate = 0;

    // set the device that this user client is to talk to
    iDevice = OSDynamicCast(AudioDevice, aProvider);
    if (!iDevice) {
        IOLog("Songcast AudioUserClient[%p]::start(%p) null device failure\n", this, aProvider);
        goto Error;
    }

    // get the work loop from the device
    iWorkLoop = iDevice->getWorkLoop();
    if (!iWorkLoop) {
        IOLog("Songcast AudioUserClient[%p]::start(%p) null work group failure\n", this, aProvider);
        goto Error;
    }
    iWorkLoop->retain();

    // create the command gate
    iCommandGate = IOCommandGate::commandGate(this);
    if (!iCommandGate) {
        IOLog("Songcast AudioUserClient[%p]::start(%p) null command gate failure\n", this, aProvider);
        goto Error;
    }

    // add the command gate to the work loop
    if (iWorkLoop->addEventSource(iCommandGate) != kIOReturnSuccess) {
        IOLog("Songcast AudioUserClient[%p]::start(%p) failed to add command gate to work loop\n", this, aProvider);
        goto Error;
    };

    // make sure base class start is called after all other things that can fail
    if (!IOUserClient::start(aProvider)) {
        IOLog("Songcast AudioUserClient[%p]::start(%p) base class start failed\n", this, aProvider);
        iWorkLoop->removeEventSource(iCommandGate);
        goto Error;
    }

    IOLog("Songcast AudioUserClient[%p]::start(%p) ok\n", this, aProvider);
    return true;

Error:
    if (iCommandGate) {
        iCommandGate->release();
        iCommandGate = 0;
    }
    if (iWorkLoop) {
        iWorkLoop->release();
        iWorkLoop = 0;
    }
    iDevice = 0;
    return false;
}


void AudioUserClient::stop(IOService* aProvider)
{
    IOLog("Songcast AudioUserClient[%p]::stop(%p)\n", this, aProvider);
    IOUserClient::stop(aProvider);

    if (iCommandGate) {
        iWorkLoop->removeEventSource(iCommandGate);
        iCommandGate->release();
        iCommandGate = 0;
        iWorkLoop->release();
        iWorkLoop = 0;
        iDevice = 0;
    }
}


IOReturn AudioUserClient::clientClose()
{
    IOLog("Songcast AudioUserClient[%p]::clientClose()\n", this);

    // terminate the user client - don't call the base class clientClose()
    terminate();

    return kIOReturnSuccess;
}


IOReturn AudioUserClient::clientDied()
{
    IOLog("Songcast AudioUserClient[%p]::clientDied()\n", this);

    // the user space application has crashed - get the driver to stop sending audio
    if (DeviceOk() == kIOReturnSuccess)
    {
        iDevice->GetSongcast().SetActive(false);
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


IOReturn AudioUserClient::RunAction(Action aAction, void* aArg1, void* aArg2, void* aArg3)
{
    // run the action using the command gate - this means the code in aAction will be run
    // when the driver's work loop gate is closed i.e. provides single threaded access
    if (iCommandGate) {
        return iCommandGate->runAction(ActionCommandGate, (void*)aAction, aArg1, aArg2, aArg3);
    }
    else {
        return kIOReturnNotAttached;
    }
}


IOReturn AudioUserClient::ActionCommandGate(OSObject* aOwner, void* aArg1, void* aArg2, void* aArg3, void* aArg4)
{
    if (!aOwner) {
        return kIOReturnBadArgument;
    }

    AudioUserClient* client = OSDynamicCast(AudioUserClient, aOwner);
    if (!client) {
        return kIOReturnBadArgument;
    }

    Action action = (Action)aArg1;
    if (!action) {
        return kIOReturnBadArgument;
    }

    return (*action)(client, aArg2, aArg3, aArg4);
}


#define ARG_UINT64(arg) *((uint64_t*)(arg))


// eOpen

IOReturn AudioUserClient::Open()
{
    return RunAction(ActionOpen);
}

IOReturn AudioUserClient::ActionOpen(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoOpen();
}

IOReturn AudioUserClient::DoOpen()
{
    if (iDevice == 0 || isInactive()) {
        IOLog("Songcast AudioUserClient[%p]::Open() not attached\n", this);
        return kIOReturnNotAttached;
    }
    else if (!iDevice->open(this)) {
        IOLog("Songcast AudioUserClient[%p]::Open() exclusive access\n", this);
        return kIOReturnExclusiveAccess;
    }

    IOLog("Songcast AudioUserClient[%p]::Open() ok\n", this);
    return kIOReturnSuccess;
}


// eClose

IOReturn AudioUserClient::Close()
{
    return RunAction(ActionClose);
}

IOReturn AudioUserClient::ActionClose(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoClose();
}

IOReturn AudioUserClient::DoClose()
{
    IOReturn ret = DeviceOk();
    if (ret != kIOReturnSuccess) {
        IOLog("Songcast AudioUserClient[%p]::Close() returns %x\n", this, ret);
        return ret;
    }

    iDevice->close(this);
    IOLog("Songcast AudioUserClient[%p]::Close() ok\n", this);
    return kIOReturnSuccess;
}


// eSetActive

IOReturn AudioUserClient::SetActive(uint64_t aActive)
{
    return RunAction(ActionSetActive, &aActive);
}

IOReturn AudioUserClient::ActionSetActive(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoSetActive(ARG_UINT64(aArg1));
}

IOReturn AudioUserClient::DoSetActive(uint64_t aActive)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->GetSongcast().SetActive(aActive);
    }
    else {
        IOLog("Songcast AudioUserClient[%p]::SetActive(%llu) returns %x\n", this, aActive, ret);
    }
    return ret;
}


// eSetEndpoint

IOReturn AudioUserClient::SetEndpoint(uint64_t aIpAddress, uint64_t aPort, uint64_t aAdapter)
{
    return RunAction(ActionSetEndpoint, &aIpAddress, &aPort, &aAdapter);
}

IOReturn AudioUserClient::ActionSetEndpoint(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoSetEndpoint(ARG_UINT64(aArg1), ARG_UINT64(aArg2), ARG_UINT64(aArg3));
}

IOReturn AudioUserClient::DoSetEndpoint(uint64_t aIpAddress, uint64_t aPort, uint64_t aAdapter)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->GetSongcast().SetEndpoint(aIpAddress, aPort, aAdapter);
    }
    else {
        IOLog("Songcast AudioUserClient[%p]::SetEndpoint(%llu, %llu, %llu) returns %x\n", this, aIpAddress, aPort, aAdapter, ret);
    }
    return ret;
}


// eSetTtl

IOReturn AudioUserClient::SetTtl(uint64_t aTtl)
{
    return RunAction(ActionSetTtl, &aTtl);
}

IOReturn AudioUserClient::ActionSetTtl(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoSetTtl(ARG_UINT64(aArg1));
}

IOReturn AudioUserClient::DoSetTtl(uint64_t aTtl)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->GetSongcast().SetTtl(aTtl);
    }
    else {
        IOLog("Songcast AudioUserClient[%p]::SetTtl(%llu) returns %x\n", this, aTtl, ret);
    }
    return ret;
}


// eSetLatencyMs

IOReturn AudioUserClient::SetLatencyMs(uint64_t aLatencyMs)
{
    return RunAction(ActionSetLatencyMs, &aLatencyMs);
}

IOReturn AudioUserClient::ActionSetLatencyMs(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoSetLatencyMs(ARG_UINT64(aArg1));
}

IOReturn AudioUserClient::DoSetLatencyMs(uint64_t aLatencyMs)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess) {
        iDevice->Engine().SetLatencyMs(aLatencyMs);
    }
    else {
        IOLog("Songcast AudioUserClient[%p]::SetLatencyMs(%llu) returns %x\n", this, aLatencyMs, ret);
    }
    return ret;
}


// eResend

IOReturn AudioUserClient::Resend(uint64_t aFrameCount, const uint32_t* aFrames)
{
    return RunAction(ActionResend, &aFrameCount, (void*)aFrames);
}

IOReturn AudioUserClient::ActionResend(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3)
{
    return aClient->DoResend(ARG_UINT64(aArg1), (const uint32_t*)aArg2);
}

IOReturn AudioUserClient::DoResend(uint64_t aFrameCount, const uint32_t* aFrames)
{
    IOReturn ret = DeviceOk();
    if (ret == kIOReturnSuccess)
    {
        IOLog("Songcast AudioUserClient[%p]::Resend(%llu, [", this, aFrameCount, ret);

        for (uint64_t i=0 ; i<aFrameCount ; i++)
        {
            uint32_t frame = OSSwapBigToHostInt32(*(aFrames + i));
            IOLog("%u,", frame);
        }
        IOLog("])\n");
    }
    else {
        IOLog("Songcast AudioUserClient[%p]::Resend(%llu) returns %x\n", this, aFrameCount, ret);
    }
    return ret;
}



