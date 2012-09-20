
#import "ModelSongcast.h"
#import "Preferences.h"
#include "../../Songcast.h"


// Declaration for receiver callback - defined in ReceiverList.mm
extern void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver);

// Declaration for subnet callback - defined in SubnetList.mm
extern void SubnetListCallback(void* aPtr, ECallbackType aType, THandle aSubnet);

// Forward declarations of callback functions defined below
void ModelConfigurationChangedCallback(void* aPtr, THandle aSongcast);
void ModelFatalErrorCallback(void* aPtr, const char* aMsg);
void ModelLogOutputCallback(void* aPtr, const char* aMsg);


@implementation ModelSongcast


- (id) initWithReceivers:(NSArray*)aReceivers andSelectedUdns:(NSArray*)aSelectedUdns multicastEnabled:(bool)aMulticastEnabled multicastChannel:(uint32_t)aMulticastChannel latencyMs:(uint32_t)aLatencyMs subnet:(uint32_t)aSubnet
{
    [super init];

    iReceiversChangedObject = 0;
    iReceiversChangedSelector = 0;
    iSubnetsChangedObject = 0;
    iSubnetsChangedSelector = 0;
    iConfigurationChangedObject = 0;
    iConfigurationChangedSelector = 0;

    // build list of initial receivers
    NSMutableArray* receivers = [NSMutableArray arrayWithCapacity:0];
    for (PrefReceiver* pref in aReceivers)
    {
        [receivers addObject:[[[Receiver alloc] initWithPref:pref] autorelease]];
    }

    // create the lists
    iReceivers = [[ReceiverList alloc] initWithReceivers:receivers];
    iSelectedUdns = [aSelectedUdns retain];
    iSubnets = [[SubnetList alloc] init];

    // setup observer for the lists
    [iReceivers setObserver:self];
    [iSubnets setObserver:self];

    // create the songcast object - always create disabled
    uint32_t ttl = 4;
    uint32_t enabled = 0;
    uint32_t preset = 0;
    NSString* domain = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastDomain"];
    NSString* manufacturerName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastManufacturerName"];
    NSString* manufacturerUrl = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastManufacturerUrl"];
    NSString* modelUrl = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastModelUrl"];

    NSString* senderIconFile = [[NSBundle mainBundle] pathForResource:@"SenderIcon" ofType:@"png"];
    NSData* senderIcon = [NSData dataWithContentsOfFile:senderIconFile];

    iSongcast = SongcastCreate([domain UTF8String], aSubnet, aMulticastChannel, ttl, aLatencyMs, aMulticastEnabled ? 1: 0, enabled, preset, ReceiverListCallback, iReceivers, SubnetListCallback, iSubnets, ModelConfigurationChangedCallback, self, ModelFatalErrorCallback, self, ModelLogOutputCallback, self, [manufacturerName UTF8String], [manufacturerUrl UTF8String], [modelUrl UTF8String], (void*)[senderIcon bytes], [senderIcon length], "image/png");

    return self;
}


- (void) dispose
{
    // destroy the songcast object
    SongcastDestroy(iSongcast);
    iSongcast = 0;

    // update all receivers to be unavailable
    for (Receiver* receiver in [iReceivers receivers])
    {
        [receiver updateWithPtr:nil];
    }
    [iReceiversChangedObject performSelector:iReceiversChangedSelector withObject:nil];

    // clear event handlers
    iReceiversChangedObject = 0;
    iReceiversChangedSelector = 0;
    iSubnetsChangedObject = 0;
    iSubnetsChangedSelector = 0;
    iConfigurationChangedObject = 0;
    iConfigurationChangedSelector = 0;

    [iReceivers setObserver:nil];
    [iSubnets setObserver:nil];
}


- (void) dealloc
{
    [iReceivers release];
    [iSelectedUdns release];
    [iSubnets release];

    [super dealloc];
}


- (void) setReceiversChangedObserver:(id)aObserver selector:(SEL)aSelector
{
    iReceiversChangedObject = aObserver;
    iReceiversChangedSelector = aSelector;
}


- (void) setSubnetsChangedObserver:(id)aObserver selector:(SEL)aSelector
{
    iSubnetsChangedObject = aObserver;
    iSubnetsChangedSelector = aSelector;
}


- (void) setConfigurationChangedObserver:(id)aObserver selector:(SEL)aSelector
{
    iConfigurationChangedObject = aObserver;
    iConfigurationChangedSelector = aSelector;
}


- (bool) enabled
{
    return (SongcastEnabled(iSongcast) != 0);
}


- (void) setEnabled:(bool)aValue
{
    // stop receivers before disabling songcast
    if (!aValue)
    {
        [self stopReceivers];
    }

    SongcastSetEnabled(iSongcast, aValue ? 1 : 0);

    // start receivers after songcast is enabled
    if (aValue)
    {
        // On switching on songcast, play receivers that are in the group
        [self playReceivers];
    }
}


- (void) setMulticastEnabled:(bool)aValue
{
    SongcastSetMulticast(iSongcast, aValue ? 1 : 0);
}


- (void) setMulticastChannel:(uint32_t)aValue
{
    SongcastSetChannel(iSongcast, aValue);
}


- (void) setLatencyMs:(uint32_t)aValue
{
    SongcastSetLatency(iSongcast, aValue);
}


- (NSArray*) receivers
{
    return [iReceivers receivers];
}


- (void) setSelectedUdns:(NSArray*)aSelectedUdns
{
    // if songcast is enabled, need to play/stop receivers that have been
    // selected/deselected
    if ([self enabled])
    {
        // build the list of receivers that have just been deselected
        NSMutableArray* deselected = [NSMutableArray arrayWithCapacity:0];
        for (NSString* udn in iSelectedUdns)
        {
            if (![aSelectedUdns containsObject:udn])
            {
                [deselected addObject:udn];
            }
        }

        // build the list of receivers that have just been selected
        NSMutableArray* selected = [NSMutableArray arrayWithCapacity:0];
        for (NSString* udn in aSelectedUdns)
        {
            if (![iSelectedUdns containsObject:udn])
            {
                [selected addObject:udn];
            }
        }

        // now play and stop the relevant receivers
        for (Receiver* receiver in [iReceivers receivers])
        {
            if ([deselected containsObject:[receiver udn]])
            {
                [self stopReceiver:receiver];
            }
        }

        for (Receiver* receiver in [iReceivers receivers])
        {
            if ([selected containsObject:[receiver udn]])
            {
                [self playReceiver:receiver];
            }
        }
    }

    // now discard the old list
    [iSelectedUdns release];
    iSelectedUdns = [aSelectedUdns retain];
}


- (void) refreshReceivers
{
    // remove undiscovered, unselected receivers
    [iReceivers removeUnavailableUnselected:iSelectedUdns];

    // notify the upper layers
    [iReceiversChangedObject performSelector:iReceiversChangedSelector withObject:nil];

    // signal songcast to refresh
    SongcastRefreshReceivers(iSongcast);
}


- (void) stopReceiver:(Receiver*)aReceiver
{
    switch ([aReceiver status])
    {
        case eReceiverStateOffline:
        case eReceiverStateDisconnected:
            // These states imply that there has been some sort of external interaction
            // with the receiver, such as changing source using Kinsky, so do not
            // tamper with them
            break;

        case eReceiverStateConnecting:
        case eReceiverStateConnected:
            // These states imply the receiver is still connected to this songcast
            // sender, so stop them and put them into standby
            [aReceiver stop];
            [aReceiver standby];
            break;
    }
}


- (void) stopReceivers
{
    for (Receiver* receiver in [iReceivers receivers])
    {
        if ([iSelectedUdns containsObject:[receiver udn]])
        {
            [self stopReceiver:receiver];
        }
    }
}


- (void) playReceiver:(Receiver*)aReceiver
{
    // Only set the receiver to playing if it is disconnected - all other states imply that
    // the receiver is already playing or unavailable
    if ([aReceiver status] == eReceiverStateDisconnected)
    {
        [aReceiver play];
    }
}


- (void) playReceivers
{
    if (![self enabled])
        return;

    for (Receiver* receiver in [iReceivers receivers])
    {
        if ([iSelectedUdns containsObject:[receiver udn]])
        {
            [self playReceiver:receiver];
        }
    }
}


- (NSArray*) subnets
{
    return [iSubnets subnets];
}


- (uint32_t) subnet
{
    return SongcastSubnet(iSongcast);
}


- (void) setSubnet:(uint32_t)aAddress
{
    SongcastSetSubnet(iSongcast, aAddress);
}


- (void) receiverAdded:(Receiver*)aReceiver
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // the receiver has just appeared on the network - start playing if required i.e.
    //  - songcast is switched on
    //  - receiver is selected
    if ([self enabled] && [iSelectedUdns containsObject:[aReceiver udn]])
    {
        [self playReceiver:aReceiver];
    }

    // notify upper layers
    [iReceiversChangedObject performSelector:iReceiversChangedSelector withObject:nil];
}


- (void) receiverRemoved:(Receiver*)aReceiver
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // notify upper layers
    [iReceiversChangedObject performSelector:iReceiversChangedSelector withObject:nil];
}


- (void) receiverChanged:(Receiver*)aReceiver
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // notify upper layers
    [iReceiversChangedObject performSelector:iReceiversChangedSelector withObject:nil];
}


- (void) subnetAdded:(Subnet*)aSubnet
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // notify upper layers
    [iSubnetsChangedObject performSelector:iSubnetsChangedSelector withObject:nil];
}


- (void) subnetRemoved:(Subnet*)aSubnet
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // notify upper layers
    [iSubnetsChangedObject performSelector:iSubnetsChangedSelector withObject:nil];
}


- (void) subnetChanged:(Subnet*)aSubnet
{
    // do nothing if songcast has been disposed
    if (!iSongcast)
        return;

    // notify upper layers
    [iSubnetsChangedObject performSelector:iSubnetsChangedSelector withObject:nil];
}


- (void) configurationChanged
{
    // notify upper layers
    [iConfigurationChangedObject performSelector:iConfigurationChangedSelector withObject:nil];
}


@end



void ModelConfigurationChangedCallback(void* aPtr, THandle aSongcast)
{
    ModelSongcast* model = (ModelSongcast*)aPtr;
    [model configurationChanged];
}


void ModelFatalErrorCallback(void* aPtr, const char* aMsg)
{
    [NSException raise:@"SongcastError" format:@"%s", aMsg];
}


void ModelLogOutputCallback(void* aPtr, const char* aMsg)
{
}




