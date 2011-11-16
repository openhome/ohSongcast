
#import "Model.h"
#import "Receiver.h"
#include "../../Songcaster.h"


// Implementation of the model class
@implementation Model


- (id) init
{
    [super init];
    
    iObserver = nil;
    iModelSongcaster = nil;

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

    return self;
}


- (void) start
{
    if (iModelSongcaster)
        return;

    // make sure preferences are synchronised so the songcaster is correctly initialised
    [iPreferences synchronize];

    // the songcaster is always started disabled - ensure that the preferences reflect this
    [iPreferences setEnabled:false];

    // create the songcaster model
    iModelSongcaster = [[ModelSongcaster alloc] initWithReceivers:[iPreferences receiverList] andSelectedUdns:[iPreferences selectedUdnList] multicastEnabled:[iPreferences multicastEnabled] multicastChannel:[iPreferences multicastChannel] latencyMs:[iPreferences latencyMs]];
    [iModelSongcaster setReceiversChangedObserver:self selector:@selector(receiversChanged)];
    [iModelSongcaster setSubnetsChangedObserver:self selector:@selector(subnetsChanged)];
    [iModelSongcaster setConfigurationChangedObserver:self selector:@selector(configurationChanged)];
}


- (void) stop
{
    if (!iModelSongcaster)
        return;

    // disable the songcaster before destroying the songcaster - make sure the preferences reflect this and
    // the songcaster is disabled synchronously - if [self setEnabled:false] was called, the songcaster
    // would get disabled asynchronously, by which time it would have been destroyed and, therefore, the
    // receivers will not be put into standby
    [iModelSongcaster setEnabled:false];
    [iPreferences setEnabled:false];

    // dispose of the songcaster model before releasing - this will shutdown the
    // songcaster
    [iModelSongcaster dispose];

    // shutdown the songcaster
    [iModelSongcaster release];
    iModelSongcaster = 0;
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
    // then cause the state of the songcaster to be updated
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


- (bool) betaUpdatesEnabled
{
    return [iPreferences betaUpdatesEnabled];
}


- (void) reconnectReceivers
{
    if (iModelSongcaster) {
        [iModelSongcaster playReceivers];
    }
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // enable/disable the songcaster
    if (iModelSongcaster) {
        [iModelSongcaster setEnabled:[iPreferences enabled]];
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

    // set the selected list in the songcaster
    if (iModelSongcaster) {
        [iModelSongcaster setSelectedUdns:[iPreferences selectedUdnList]];
    }
}


- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification
{
    if (iModelSongcaster) {
        [iModelSongcaster refreshReceivers];
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

    if (iModelSongcaster) {
        [iModelSongcaster setMulticastEnabled:[iPreferences multicastEnabled]];
    }
}


- (void) preferenceMulticastChannelChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcaster) {
        [iModelSongcaster setMulticastChannel:[iPreferences multicastChannel]];
    }
}


- (void) preferenceLatencyMsChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcaster) {
        [iModelSongcaster setLatencyMs:[iPreferences latencyMs]];
    }
}


- (void) preferenceSelectedSubnetChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    if (iModelSongcaster) {
        [iModelSongcaster setSubnet:[[iPreferences selectedSubnet] address]];
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
    if (!iModelSongcaster)
        return;

    // build a new list of receivers to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Receiver* receiver in [iModelSongcaster receivers])
    {
        [list addObject:[receiver convertToPref]];
    }
    
    // set - this sends notification of the change
    [iPreferences setReceiverList:list];
}


- (void) subnetsChanged
{
    if (!iModelSongcaster)
        return;

    // build a new list of subnets to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Subnet* subnet in [iModelSongcaster subnets])
    {
        [list addObject:[subnet convertToPref]];
    }

    // set - this sends notification of the change
    [iPreferences setSubnetList:list];

    // get the currently selected subnet
    PrefSubnet* selected = [iPreferences selectedSubnet];

    if (selected)
    {
        if ([selected address] != [iModelSongcaster subnet])
        {
            // set the subnet to the selected subnet if it is available
            for (Subnet* subnet in [iModelSongcaster subnets])
            {
                if ([subnet address] == [selected address])
                {
                    [iModelSongcaster setSubnet:[subnet address]];
                }
            }
        }
    }
    else
    {
        // no selected subnet in the preferences
        if ([iModelSongcaster subnet] == 0)
        {
            // songcaster currently has no subnet
            if ([[iModelSongcaster subnets] count] != 0)
            {
                // set the subnet to the first in list
                Subnet* subnet = [[iModelSongcaster subnets] objectAtIndex:0];
                [iModelSongcaster setSubnet:[subnet address]];

                // update the preference
                [iPreferences setSelectedSubnet:[subnet convertToPref]];
            }
        }
    }
}


- (void) configurationChanged
{
    if (!iModelSongcaster)
        return;

    [self setEnabled:[iModelSongcaster enabled]];
}


@end



