
#import "Model.h"
#import "Receiver.h"
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
    iEnabled = false;
    iAutoplay = true;
    iReceiverList = nil;
    iSelectedUdnList = nil;
    iObserver = nil;

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
    
    // get other preference data
    iEnabled = [iPreferences enabled];
    iAutoplay = [iPreferences autoplayReceivers];
    iSelectedUdnList = [[iPreferences selectedUdnList] retain];

    // setup some event handlers
    [iReceiverList addObserver:self];
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    [iPreferences addObserverIconVisible:self selector:@selector(preferenceIconVisibleChanged:)];
    [iPreferences addObserverSelectedUdnList:self selector:@selector(preferenceSelectedUdnListChanged:)];
    [iPreferences addObserverAutoplayReceivers:self selector:@selector(preferenceAutoplayReceiversChanged:)];
    
    // create the soundcard object
    uint32_t subnet = 0;
    uint32_t channel = 0;
    uint32_t ttl = 4;
    uint32_t multicast = 0;
    uint32_t preset = 0;
    iSoundcard = SoundcardCreate(subnet, channel, ttl, multicast, iEnabled ? 1 : 0, preset, ReceiverListCallback, iReceiverList, ModelSubnetCallback, self);
}


- (void) playReceiver:(Receiver*)aReceiver
{
    // Start playing a receiver only if it is currently disconnected
    if ([aReceiver status] == eReceiverStateDisconnected)
    {
        [aReceiver play];
    }
}


- (void) stopReceiver:(Receiver*)aReceiver
{
    // A receiver is stopped and put into standby only if it is connected to this soundcard
    // If the receiver is disconnected, this implies there has been some sort
    // of interaction with that receiver from another control point, e.g. change of
    // source, so these receivers are not tampered with
    if ([aReceiver status] == eReceiverStateConnected || [aReceiver status] == eReceiverStateConnecting)
    {
        [aReceiver stop];
//        [aReceiver standby];
    }
}


- (void) stopReceivers
{
    // Only stop receivers that are in the selected list
    for (Receiver* receiver in [iReceiverList receivers])
    {
        if ([iSelectedUdnList containsObject:[receiver udn]])
        {
            [self stopReceiver:receiver];
        }
    }
}


- (void) stop
{
    // soundcard app shutting down - stop receivers before destroying soundcard
    [self stopReceivers];

    // shutdown the soundcard
    SoundcardDestroy(iSoundcard);
    iSoundcard = NULL;

    [iSelectedUdnList release];
    [iReceiverList release];
    [iPreferences release];
}


- (void) setObserver:(id<IModelObserver>)aObserver
{
    iObserver = aObserver;
}


- (bool) iconVisible
{
    return [iPreferences iconVisible];
}


- (bool) enabled
{
    return iEnabled;
}


- (void) setEnabled:(bool)aValue
{
    // just set the preference - eventing by the preference change will
    // then cause the state of the soundcard to be updated
    [iPreferences setEnabled:aValue];
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences and update the local copy
    [iPreferences synchronize];
    iEnabled = [iPreferences enabled];

    // stop receivers before disabling soundcard
    if (!iEnabled)
    {
        [self stopReceivers];
    }

    // enable/disable the soundcard
    SoundcardSetEnabled(iSoundcard, iEnabled ? 1 : 0);

    // start receivers after soundcard is enabled
    if (iEnabled)
    {
        // On switching on the soundcard, if autoplay is enabled, all disconnected receivers
        // are played. If autoplay is disabled, nothing happens.
        if (iAutoplay)
        {
            for (Receiver* receiver in [iReceiverList receivers])
            {
                if ([iSelectedUdnList containsObject:[receiver udn]])
                {
                    [self playReceiver:receiver];
                }
            }
        }
    }
    
    // notify UI
    [iObserver enabledChanged];    
}


- (void) preferenceIconVisibleChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];
    
    // notify UI
    [iObserver iconVisibleChanged];
}


- (void) preferenceSelectedUdnListChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // get the new list of selected receivers in order to determine changes in the list
    NSArray* newSelectedUdnList = [[iPreferences selectedUdnList] retain];

    // if the soundcard is enabled, need to play/stop receivers that have been
    // selected/deselected
    if (iEnabled)
    {
        // build the list of receivers that have just been deselected
        NSMutableArray* deselected = [NSMutableArray arrayWithCapacity:0];
        for (NSString* udn in iSelectedUdnList)
        {
            if (![newSelectedUdnList containsObject:udn])
            {
                [deselected addObject:udn];
            }
        }
    
        // build the list of receivers that have just been selected
        NSMutableArray* selected = [NSMutableArray arrayWithCapacity:0];
        for (NSString* udn in newSelectedUdnList)
        {
            if (![iSelectedUdnList containsObject:udn])
            {
                [selected addObject:udn];
            }
        }
        
        // now play and stop the relevant receivers
        for (Receiver* receiver in [iReceiverList receivers])
        {
            if ([deselected containsObject:[receiver udn]])
            {
                [self stopReceiver:receiver];
            }
            else if ([selected containsObject:[receiver udn]])
            {
                [self playReceiver:receiver];
            }
        }
    }
    
    // now discard the old list
    [iSelectedUdnList release];
    iSelectedUdnList = newSelectedUdnList;
}


- (void) preferenceAutoplayReceiversChanged:(NSNotification*)aNotification
{
    // refresh cached preferences and update the local copy
    [iPreferences synchronize];
    iAutoplay = [iPreferences autoplayReceivers];
}


- (void) updatePreferenceReceiverList
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


- (void) receiverAdded:(Receiver *)aReceiver
{
    [self updatePreferenceReceiverList];

    // the receiver has just appeared on the network - start playing if required i.e.
    //  - autoplay option is enabled
    //  - soundcard is switched on
    //  - receiver is selected
    if (iAutoplay &&
        iEnabled &&
        [iSelectedUdnList containsObject:[aReceiver udn]])
    {
        [self playReceiver:aReceiver];
    }
}


- (void) receiverRemoved:(Receiver *)aReceiver
{
    [self updatePreferenceReceiverList];
}


- (void) receiverChanged:(Receiver *)aReceiver
{
    [self updatePreferenceReceiverList];
}


@end



// Callbacks from the ohSoundcard code
void ModelSubnetCallback(void* aPtr, ECallbackType aType, THandle aSubnet)
{
}



