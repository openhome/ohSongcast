
#import "Model.h"
#include "../../Soundcard.h"


// Forward declarations of callback functions defined below
void ModelReceiverCallback(void* aPtr, ECallbackType aType, THandle aReceiver);
void ModelSubnetCallback(void* aPtr, ECallbackType aType, THandle aSubnet);



// Declaration of the receiver class
@interface Receiver : NSObject
{
    NSString* udn;
    NSString* name;
    void* iPtr;
}

@property (assign) NSString* udn;
@property (assign) NSString* name;

- (id) initWithPtr:(void*)aPtr;
- (id) initWithPref:(PrefReceiver*)aPref;
- (void) updateWithPtr:(void*)aPtr;
- (PrefReceiver*) convertToPref;

@end



// Implementation of the model class
@implementation Model


- (id) init
{
    [super init];
    
    iEnabled = FALSE;
    iSoundcard = nil;
    iPreferences = nil;
    iReceiverList = nil;

    return self;
}


- (void) start
{
    // create the preferences object
    iPreferences = [[Preferences alloc] init];
    [iPreferences synchronize];

    // create an empty receiver list
    iReceiverList = [[NSMutableArray alloc] init];

    // populate the receiver list with that stored in the preferences
    NSArray* list = [iPreferences receiverList];
    for (PrefReceiver* pref in list)
    {
        [iReceiverList addObject:[[[Receiver alloc] initWithPref:pref] autorelease]];
    }

    // create the soundcard object
    uint32_t subnet = 0;
    uint32_t channel = 0;
    uint32_t ttl = 4;
    uint32_t multicast = 0;
    uint32_t enabled = 0;
    uint32_t preset = 0;
    iSoundcard = SoundcardCreate(subnet, channel, ttl, multicast, enabled, preset, ModelReceiverCallback, self, ModelSubnetCallback, self);
}


- (void) stop
{
    SoundcardDestroy(iSoundcard);
    iSoundcard = NULL;
    
    [iReceiverList release];
    [iPreferences release];
}


- (bool) enabled
{
    return iEnabled;
}


- (void) setEnabled:(bool)aValue
{
    SoundcardSetEnabled(iSoundcard, aValue ? 1 : 0);
    iEnabled = aValue;
}


- (Receiver*) receiverWithUdn:(NSString*)aUdn
{
    for (Receiver* receiver in iReceiverList)
    {
        if ([[receiver udn] compare:aUdn] == NSOrderedSame)
        {
            return receiver;
        }
    }
    
    return nil;
}


- (void) updatePrefsReceiverList
{
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];

    for (Receiver* receiver in iReceiverList)
    {
        [list addObject:[receiver convertToPref]];
    }
    
    [iPreferences setReceiverList:list];
    [iPreferences synchronize];
}


- (void) receiverAdded:(void*)aReceiver
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn(aReceiver)];

    Receiver* receiver = [self receiverWithUdn:udn];

    // update existing receivers and add new ones
    if (receiver)
    {
        [receiver updateWithPtr:aReceiver];
    }
    else
    {
        receiver = [[[Receiver alloc] initWithPtr:aReceiver] autorelease];
        [iReceiverList addObject:receiver];
    }

    // update the preferences
    [self updatePrefsReceiverList];
}


- (void) receiverRemoved:(void*)aReceiver
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn(aReceiver)];
    
    Receiver* receiver = [self receiverWithUdn:udn];

    if (receiver)
    {
        // receiver is in the list - clear the pointer
        [receiver updateWithPtr:nil];

        // update the preferences
        [self updatePrefsReceiverList];
    }
}


- (void) receiverChanged:(void*)aReceiver
{
    [self receiverAdded:aReceiver];
}


- (void) subnetAdded:(void*)aSubnet
{
}

- (void) subnetRemoved:(void*)aSubnet
{
}

- (void) subnetChanged:(void*)aSubnet
{
}


@end



// Callbacks from the ohSoundcard code
void ModelReceiverCallback(void* aPtr, ECallbackType aType, THandle aReceiver)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    Model* model = (Model*)aPtr;
    switch (aType)
    {
        case eAdded:
            [model receiverAdded:aReceiver];
            break;
        case eRemoved:
            [model receiverRemoved:aReceiver];
            break;
        case eChanged:
            [model receiverChanged:aReceiver];
            break;
    }
    
    [pool drain];
}

void ModelSubnetCallback(void* aPtr, ECallbackType aType, THandle aSubnet)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    Model* model = (Model*)aPtr;
    switch (aType)
    {
        case eAdded:
            [model subnetAdded:aSubnet];
            break;
        case eRemoved:
            [model subnetRemoved:aSubnet];
            break;
        case eChanged:
            [model subnetChanged:aSubnet];
            break;
    }
    
    [pool drain];
}



// Implementation of receiver class
@implementation Receiver

@synthesize udn;
@synthesize name;


- (id) initWithPtr:(void *)aPtr
{
    [super init];

    iPtr = aPtr;
    ReceiverAddRef(iPtr);

    udn = [[NSString alloc] initWithUTF8String:ReceiverUdn(iPtr)];
    name = [[NSString alloc] initWithUTF8String:ReceiverName(iPtr)];    
    
    return self;
}


- (id) initWithPref:(PrefReceiver*)aPref
{
    [super init];
    
    iPtr = nil;
    
    udn = [[aPref udn] retain];
    name = [[aPref name] retain];
    
    return self;
}


- (void) updateWithPtr:(void*)aPtr
{
    if (iPtr != aPtr)
    {
        if (iPtr)
            ReceiverRemoveRef(iPtr);
        if (aPtr)
            ReceiverAddRef(aPtr);
    }
    
    iPtr = aPtr;
    
    if (aPtr)
    {
        [udn release];
        [name release];
        udn = [[NSString alloc] initWithUTF8String:ReceiverUdn(iPtr)];
        name = [[NSString alloc] initWithUTF8String:ReceiverName(iPtr)];
    }
}


- (PrefReceiver*) convertToPref
{
    PrefReceiver* pref = [[[PrefReceiver alloc] init] autorelease];
    
    [pref setUdn:udn];
    [pref setName:name];
    [pref setAvailable:(iPtr != nil)];
    
    return pref;
}


@end



