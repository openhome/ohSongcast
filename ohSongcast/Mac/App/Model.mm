
#import "Model.h"
#import "Receiver.h"
#include "../../Songcast.h"


// Implementation of the model class
@implementation Model


- (id) init
{
    [super init];
    
    iObserver = nil;
    iModelSongcast = nil;

    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[NSBundle mainBundle]];
    [iPreferences synchronize];

    // setup some event handlers
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    [iPreferences addObserverIconVisible:self selector:@selector(preferenceIconVisibleChanged:)];
    [iPreferences addObserverSelectedUdnList:self selector:@selector(preferenceSelectedUdnListChanged:)];
    [iPreferences addObserverRefreshReceiverList:self selector:@selector(preferenceRefreshReceiverList:)];
    [iPreferences addObserverReconnectReceivers:self selector:@selector(preferenceReconnectReceivers:)];
    [iPreferences addObserverMulticastEnabled:self selector:@selector(preferenceMulticastEnabledChanged:)];
    [iPreferences addObserverMulticastChannel:self selector:@selector(preferenceMulticastChannelChanged:)];
    [iPreferences addObserverLatencyMs:self selector:@selector(preferenceLatencyMsChanged:)];
    [iPreferences addObserverSelectedSubnet:self selector:@selector(preferenceSelectedSubnetChanged:)];
    [iPreferences addObserverAutoUpdatesEnabled:self selector:@selector(preferenceAutoUpdatesEnabledChanged:)];
    [iPreferences addObserverBetaUpdatesEnabled:self selector:@selector(preferenceBetaUpdatesEnabledChanged:)];
    [iPreferences addObserverCheckForUpdates:self selector:@selector(preferenceCheckForUpdates:)];

    // create the auto updater object
    NSString* productId = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastProductId"];
    NSString* version = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    NSString* autoUpdateUrl = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"SongcastAutoUpdateUrl"];

    if ([autoUpdateUrl length] != 0)
    {
        iAutoUpdate = [[AutoUpdate alloc] initWithFeedUri:autoUpdateUrl
                                          appName:productId
                                          currentVersion:version
                                          relativeDataPath:productId];
        [iAutoUpdate setCheckForBeta:[iPreferences betaUpdatesEnabled]];
    }

    return self;
}


- (void) dealloc
{
    [self stop];
    [iPreferences release];
    [iAutoUpdate release];
    [super dealloc];
}


- (void) start
{
    if (iModelSongcast)
        return;

    // make sure preferences are synchronised so songcast is correctly initialised
    [iPreferences synchronize];

    // songcast is always started disabled - ensure that the preferences reflect this
    [iPreferences setEnabled:false];

    // get the selected subnet stored in the preferences
    PrefSubnet* selectedSubnet = [iPreferences selectedSubnet];
    uint32_t subnet = (selectedSubnet != NULL) ? [selectedSubnet address] : 0;

    // create the songcast model
    iModelSongcast = [[ModelSongcast alloc] initWithReceivers:[iPreferences receiverList] andSelectedUdns:[iPreferences selectedUdnList] multicastEnabled:[iPreferences multicastEnabled] multicastChannel:[iPreferences multicastChannel] latencyMs:[iPreferences latencyMs] subnet:subnet];
    [iModelSongcast setReceiversChangedObserver:self selector:@selector(receiversChanged)];
    [iModelSongcast setSubnetsChangedObserver:self selector:@selector(subnetsChanged)];
    [iModelSongcast setConfigurationChangedObserver:self selector:@selector(configurationChanged)];
}


- (void) stop
{
    if (!iModelSongcast)
        return;

    // disable songcast before destroying songcast - make sure the preferences reflect this and
    // songcast is disabled synchronously - if [self setEnabled:false] was called, songcast
    // would get disabled asynchronously, by which time it would have been destroyed and, therefore, the
    // receivers will not be put into standby
    [iModelSongcast setEnabled:false];
    [iPreferences setEnabled:false];

    // dispose of the songcast model before releasing - this will shutdown
    // songcast
    [iModelSongcast dispose];

    // shutdown songcast
    [iModelSongcast release];
    iModelSongcast = 0;
}


- (void) setObserver:(id<IModelObserver>)aObserver
{
    iObserver = aObserver;

    // send events to update the observer
    [iObserver enabledChanged];    
    [iObserver iconVisibleChanged];
}


- (bool) iconVisible
{
    return [iPreferences iconVisible];
}


- (bool) enabled
{
    return [iPreferences enabled];
}


- (void) setEnabled:(bool)aValue
{
    // just set the preference - eventing by the preference change will
    // then cause the state of songcast to be updated
    [iPreferences setEnabled:aValue];
}


- (bool) hasRunWizard
{
    return [iPreferences hasRunWizard];
}


- (bool) autoUpdatesEnabled
{
    return [iPreferences autoUpdatesEnabled];
}


- (AutoUpdate*) autoUpdate
{
    return iAutoUpdate;
}


- (void) reconnectReceivers
{
    if (iModelSongcast) {
        [iModelSongcast playReceivers];
    }
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // enable/disable songcast
    if (iModelSongcast) {
        [iModelSongcast setEnabled:[iPreferences enabled]];
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

    // set the selected list in songcast
    if (iModelSongcast) {
        [iModelSongcast setSelectedUdns:[iPreferences selectedUdnList]];
    }
}


- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification
{
    if (iModelSongcast) {
        [iModelSongcast refreshReceivers];
    }
}


- (void) preferenceReconnectReceivers:(NSNotification*)aNotification
{
    [self reconnectReceivers];
}


- (void) preferenceMulticastEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcast) {
        [iModelSongcast setMulticastEnabled:[iPreferences multicastEnabled]];
    }
}


- (void) preferenceMulticastChannelChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcast) {
        [iModelSongcast setMulticastChannel:[iPreferences multicastChannel]];
    }
}


- (void) preferenceLatencyMsChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcast) {
        [iModelSongcast setLatencyMs:[iPreferences latencyMs]];
    }
}


- (void) preferenceSelectedSubnetChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcast) {
        [iModelSongcast setSubnet:[[iPreferences selectedSubnet] address]];
    }
}


- (void) preferenceAutoUpdatesEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // trigger a check for updates if they have become enabled
    if ([iPreferences autoUpdatesEnabled]) {
        [iObserver checkForUpdates];
    }
}


- (void) preferenceBetaUpdatesEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // set the parameter in the auto update object
    [iAutoUpdate setCheckForBeta:[iPreferences betaUpdatesEnabled]];

    // trigger a check for updates if they have become enabled
    if ([iPreferences autoUpdatesEnabled] && [iPreferences betaUpdatesEnabled]) {
        [iObserver checkForUpdates];
    }
}


- (void) preferenceCheckForUpdates:(NSNotification*)aNotification
{
    // notify the UI to trigger the check for updates
    [iObserver checkForUpdates];
}


- (void) receiversChanged
{
    if (!iModelSongcast)
        return;

    // build a new list of receivers to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Receiver* receiver in [iModelSongcast receivers])
    {
        [list addObject:[receiver convertToPref]];
    }
    
    // set - this sends notification of the change
    [iPreferences setReceiverList:list];
}


- (void) subnetsChanged
{
    if (!iModelSongcast)
        return;

    // build a new list of subnets to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Subnet* subnet in [iModelSongcast subnets])
    {
        [list addObject:[subnet convertToPref]];
    }

    // set - this sends notification of the change
    [iPreferences setSubnetList:list];

    // get the currently selected subnet
    PrefSubnet* selected = [iPreferences selectedSubnet];

    if (selected)
    {
        if ([selected address] != [iModelSongcast subnet])
        {
            // set the subnet to the selected subnet if it is available
            for (Subnet* subnet in [iModelSongcast subnets])
            {
                if ([subnet address] == [selected address])
                {
                    [iModelSongcast setSubnet:[subnet address]];
                }
            }
        }
    }
    else
    {
        // no selected subnet in the preferences
        if ([iModelSongcast subnet] == 0)
        {
            // songcast currently has no subnet
            if ([[iModelSongcast subnets] count] != 0)
            {
                // set the subnet to the first in list
                Subnet* subnet = [[iModelSongcast subnets] objectAtIndex:0];
                [iModelSongcast setSubnet:[subnet address]];

                // update the preference
                [iPreferences setSelectedSubnet:[subnet convertToPref]];
            }
        }
    }
}


- (void) configurationChanged
{
    if (!iModelSongcast)
        return;

    [self setEnabled:[iModelSongcast enabled]];
}


@end



