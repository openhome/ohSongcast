
#import "Model.h"
#include "../../Soundcard.h"


// Declaration for soundcard receiver callback - defined in ReceiverList.mm
extern void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver);

// Forward declarations of callback functions defined below
void ModelSubnetCallback(void* aPtr, ECallbackType aType, THandle aSubnet);



// Implementation of the model class
@implementation Model


- (id) init
{
    [super init];
    
    iSoundcard = nil;
    iPreferences = nil;
    iReceiverList = nil;

    return self;
}


- (void) start
{
    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[NSBundle mainBundle]];
    [iPreferences synchronize];

    // build the initial list of receivers from the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    for (PrefReceiver* pref in [iPreferences receiverList])
    {
        [list addObject:[[[Receiver alloc] initWithPref:pref] autorelease]];
    }
    
    // create the receiver list
    iReceiverList = [[ReceiverList alloc] initWithReceivers:list];

    // setup some event handlers
    [iReceiverList addObserver:self];
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    
    // check the enabled preference
    bool enabled = [iPreferences enabled];
    
    // create the soundcard object
    uint32_t subnet = 0;
    uint32_t channel = 0;
    uint32_t ttl = 4;
    uint32_t multicast = 0;
    uint32_t preset = 0;
    iSoundcard = SoundcardCreate(subnet, channel, ttl, multicast, enabled ? 1 : 0, preset, ReceiverListCallback, iReceiverList, ModelSubnetCallback, self);
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
    return [iPreferences enabled];
}


- (void) setEnabled:(bool)aValue
{
    // just set the preference - eventing by the preference change will
    // then cause the state of the soundcard to be updated
    [iPreferences setEnabled:aValue];
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    [iPreferences synchronize];
    bool enabled = [iPreferences enabled];
    
    SoundcardSetEnabled(iSoundcard, enabled ? 1 : 0);
}


- (void) receiverListChanged
{
    // build a new list of receivers to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Receiver* receiver in [iReceiverList receivers])
    {
        [list addObject:[receiver convertToPref]];
    }

    // set - this sends notification of the change
    [iPreferences setReceiverList:list];
}


@end



// Callbacks from the ohSoundcard code
void ModelSubnetCallback(void* aPtr, ECallbackType aType, THandle aSubnet)
{
}



