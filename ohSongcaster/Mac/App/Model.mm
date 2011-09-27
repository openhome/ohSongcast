
#import "Model.h"
#import "Receiver.h"
#include "../../Songcaster.h"


// Implementation of the model class
@implementation Model


- (id) init
{
    [super init];
    
    iPreferences = nil;
    iObserver = nil;
    iModelSongcaster = nil;

    return self;
}


- (void) start
{
    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[NSBundle mainBundle]];
    [iPreferences synchronize];

    // create the songcaster model
    iModelSongcaster = [[ModelSongcaster alloc] initWithReceivers:[iPreferences receiverList] andSelectedUdns:[iPreferences selectedUdnList]];
    [iModelSongcaster setEnabled:[iPreferences enabled]];
    [iModelSongcaster setReceiversChangedObserver:self selector:@selector(receiversChanged)];
    [iModelSongcaster setConfigurationChangedObserver:self selector:@selector(configurationChanged)];

    // setup some event handlers
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    [iPreferences addObserverIconVisible:self selector:@selector(preferenceIconVisibleChanged:)];
    [iPreferences addObserverSelectedUdnList:self selector:@selector(preferenceSelectedUdnListChanged:)];
    [iPreferences addObserverRefreshReceiverList:self selector:@selector(preferenceRefreshReceiverList:)];
    [iPreferences addObserverReconnectReceivers:self selector:@selector(preferenceReconnectReceivers:)];
}


- (void) stop
{
    // stop the receivers before destroying the songcaster
    [iModelSongcaster stopReceivers];

    // shutdown the songcaster
    [iModelSongcaster release];
    iModelSongcaster = 0;

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
    return [iModelSongcaster enabled];
}


- (void) setEnabled:(bool)aValue
{
    // just set the preference - eventing by the preference change will
    // then cause the state of the songcaster to be updated
    [iPreferences setEnabled:aValue];
}


- (void) reconnectReceivers
{
    [iModelSongcaster playReceiversAndReconnect:true];
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    // refresh cached preferences
    [iPreferences synchronize];

    // enable/disable the songcaster
    [iModelSongcaster setEnabled:[iPreferences enabled]];

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
    [iModelSongcaster setSelectedUdns:[iPreferences selectedUdnList]];
}


- (void) preferenceRefreshReceiverList:(NSNotification*)aNotification
{
    [iModelSongcaster refreshReceivers];
}


- (void) preferenceReconnectReceivers:(NSNotification*)aNotification
{
    [self reconnectReceivers];
}


- (void) receiversChanged
{
    // build a new list of receivers to store in the preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
    
    for (Receiver* receiver in [iModelSongcaster receivers])
    {
        [list addObject:[receiver convertToPref]];
    }
    
    // set - this sends notification of the change
    [iPreferences setReceiverList:list];
}


- (void) configurationChanged
{
    [self setEnabled:[iModelSongcaster enabled]];
}


@end



