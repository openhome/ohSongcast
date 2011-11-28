
#import "Preferences.h"


@implementation PrefReceiver

@synthesize udn;
@synthesize room;
@synthesize group;
@synthesize name;
@synthesize status;


- (id) initWithDict:(NSDictionary*)aDict
{
    self = [super init];

    udn = [[aDict objectForKey:@"Udn"] retain];
    room = [[aDict objectForKey:@"Room"] retain];
    group = [[aDict objectForKey:@"Group"] retain];
    name = [[aDict objectForKey:@"Name"] retain];
    status = [[aDict objectForKey:@"Status"] intValue];
    
    return self;
}


- (NSDictionary*) convertToDict
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
            udn, @"Udn",
            room, @"Room",
            group, @"Group",
            name, @"Name",
            [NSNumber numberWithInt:status], @"Status", nil];
}

@end



@implementation PrefSubnet

@synthesize address;
@synthesize name;


- (id) initWithDict:(NSDictionary*)aDict
{
    self = [super init];

    address = [[aDict objectForKey:@"Address"] unsignedIntValue];
    name = [[aDict objectForKey:@"Name"] retain];

    return self;
}


- (NSDictionary*) convertToDict
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithUnsignedInt:address], @"Address",
            name, @"Name",
            nil];
}


@end



static bool gPrefMulticastChannelSeeded = false;

@implementation PrefMulticastChannel


+ (uint32_t) generate
{
    // generate a new random channel - the channel is the last 2 bytes of the
    // multicast IP address 239.253.x.x
    if (!gPrefMulticastChannelSeeded)
    {
        srandom(time(NULL));
        gPrefMulticastChannelSeeded = true;
    }

    // man page for random() state the function returns an integer in range [0, 2^31 - 1]
    uint64_t maxRand = (((uint64_t)1)<<31) - 1;

    // generating a random number between [0,N] is (random() * (N+1) / (maxRand+1)) - if
    // we use (random() * N / maxRand), the random number will only generate N when
    // random() returns maxRand which is a chance of 1 in (2^31 -1)

    // byte1 in range [1,254]
    uint64_t byte1 = random();
    byte1 *= 254;
    byte1 /= maxRand + 1;
    byte1 += 1;

    // byte2 in range [1,254]
    uint64_t byte2 = random();
    byte2 *= 254;
    byte2 /= maxRand + 1;
    byte2 += 1;

    return (byte1 << 8) | byte2;
}

@end



@implementation Preferences


- (id) initWithBundle:(NSBundle*)aBundle
{
    self = [super init];

    appId = (CFStringRef)[[[aBundle infoDictionary] objectForKey:@"SongcastPreferencesId"] retain];

    return self;
}


- (bool) getBoolPreference:(NSString*)aName default:(bool)aDefault
{
    CFPropertyListRef pref = CFPreferencesCopyAppValue((CFStringRef)aName, appId);

    if (pref)
    {
        if (CFGetTypeID(pref) == CFBooleanGetTypeID())
        {
            CFBooleanRef value = (CFBooleanRef)pref;
            return (value == kCFBooleanTrue);
        }

        CFRelease(pref);
    }

    return aDefault;
}


- (void) setBoolPreference:(NSString*)aName value:(bool)aValue notification:(NSString*)aNotification
{
    // set the new preference value
    if (aValue)
    {
        CFPreferencesSetAppValue((CFStringRef)aName, kCFBooleanTrue, appId);
    }
    else
    {
        CFPreferencesSetAppValue((CFStringRef)aName, kCFBooleanFalse, appId);
    }

    // flush the preferences
    CFPreferencesAppSynchronize(appId);

    // send notification that this has changed
    if (aNotification != nil)
    {
        CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
        CFNotificationCenterPostNotification(centre, (CFStringRef)aNotification, appId, NULL, TRUE);
    }
}


- (int64_t) getIntegerPreference:(NSString*)aName default:(int64_t)aDefault
{
    CFPropertyListRef pref = CFPreferencesCopyAppValue((CFStringRef)aName, appId);
    
    if (pref)
    {
        if (CFGetTypeID(pref) == CFNumberGetTypeID())
        {
            int64_t value;
            if (CFNumberGetValue((CFNumberRef)pref, kCFNumberSInt64Type, &value))
            {
                return value;
            }
        }
        
        CFRelease(pref);
    }
    
    return aDefault;
}


- (void) setIntegerPreference:(NSString*)aName value:(int64_t)aValue notification:(NSString*)aNotification
{
    // set the new preference value
    CFNumberRef pref = CFNumberCreate(NULL, kCFNumberSInt64Type, &aValue);
    CFPreferencesSetAppValue((CFStringRef)aName, pref, appId);
    CFRelease(pref);

    // flush the preferences
    CFPreferencesAppSynchronize(appId);
    
    // send notification that this has changed
    if (aNotification != nil)
    {
        CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
        CFNotificationCenterPostNotification(centre, (CFStringRef)aNotification, appId, NULL, TRUE);
    }
}


- (NSArray*) getListPreference:(NSString*)aName itemType:(CFTypeID)aItemType
{
    // create a temporary mutable list to build the list from the preference
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];

    CFPropertyListRef pref = CFPreferencesCopyAppValue((CFStringRef)aName, appId);

    if (pref)
    {
        if (CFGetTypeID(pref) == CFArrayGetTypeID())
        {
            CFArrayRef prefList = (CFArrayRef)pref;

            for (CFIndex i=0 ; i<CFArrayGetCount(prefList) ; i++)
            {
                const void* item = CFArrayGetValueAtIndex(prefList, i);

                if (item && CFGetTypeID(item) == aItemType)
                {
                    [list addObject:(NSObject*)item];
                }
            }
        }

        CFRelease(pref);
    }

    // return a new immutable array of items
    return [NSArray arrayWithArray:list];
}


- (bool) iconVisible
{
    return [self getBoolPreference:@"IconVisible" default:true];
}


- (void) setIconVisible:(bool)aVisible
{
    [self setBoolPreference:@"IconVisible" value:aVisible notification:@"PreferenceIconVisibleChanged"];
}


- (void) addObserverIconVisible:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceIconVisibleChanged" object:(NSString*)appId];
}


- (bool) enabled
{
    return [self getBoolPreference:@"Enabled" default:false];
}


- (void) setEnabled:(bool)aEnabled
{
    [self setBoolPreference:@"Enabled" value:aEnabled notification:@"PreferenceEnabledChanged"];
}


- (void) addObserverEnabled:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceEnabledChanged" object:(NSString*)appId];
}


- (bool) hasRunWizard
{
    return [self getBoolPreference:@"HasRunWizard" default:false];
}


- (void) setHasRunWizard:(bool)aHasRunWizard
{
    [self setBoolPreference:@"HasRunWizard" value:aHasRunWizard notification:nil];
}


- (bool) multicastEnabled
{
    return [self getBoolPreference:@"MulticastEnabled" default:false];
}


- (void) setMulticastEnabled:(bool)aEnabled
{
    [self setBoolPreference:@"MulticastEnabled" value:aEnabled notification:@"PreferenceMulticastEnabledChanged"];
}


- (void) addObserverMulticastEnabled:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceMulticastEnabledChanged" object:(NSString*)appId];
}


- (uint32_t) multicastChannel
{
    uint32_t defaultChannel = [PrefMulticastChannel generate];
    uint32_t channel = (uint32_t)[self getIntegerPreference:@"MulticastChannel" default:defaultChannel];

    if (channel == defaultChannel)
    {
        // channel is not current stored - the default value was returned - so set the value in the
        // preferences file - if the channel is stored and is coincidentally the same as the generated
        // default, setting it again will do no harm
        [self setMulticastChannel:channel];
    }

    return channel;
}


- (void) setMulticastChannel:(uint32_t)aChannel
{
    [self setIntegerPreference:@"MulticastChannel" value:(int64_t)aChannel notification:@"PreferenceMulticastChannelChanged"];
}


- (void) addObserverMulticastChannel:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceMulticastChannelChanged" object:(NSString*)appId];
}


- (uint32_t) latencyMs
{
    return (uint32_t)[self getIntegerPreference:@"LatencyMs" default:100];
}


- (void) setLatencyMs:(uint32_t)aLatencyMs
{
    [self setIntegerPreference:@"LatencyMs" value:(int64_t)aLatencyMs notification:@"PreferenceLatencyMsChanged"];
}


- (void) addObserverLatencyMs:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceLatencyMsChanged" object:(NSString*)appId];
}


- (bool) autoUpdatesEnabled
{
    return [self getBoolPreference:@"AutoUpdatesEnabled" default:true];
}


- (void) setAutoUpdatesEnabled:(bool)aEnabled
{
    [self setBoolPreference:@"AutoUpdatesEnabled" value:aEnabled notification:@"PreferenceAutoUpdatesEnabledChanged"];
}


- (void) addObserverAutoUpdatesEnabled:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceAutoUpdatesEnabledChanged" object:(NSString*)appId];
}


- (bool) betaUpdatesEnabled
{
    return [self getBoolPreference:@"BetaUpdatesEnabled" default:false];
}


- (void) setBetaUpdatesEnabled:(bool)aEnabled
{
    [self setBoolPreference:@"BetaUpdatesEnabled" value:aEnabled notification:@"PreferenceBetaUpdatesEnabledChanged"];
}


- (void) addObserverBetaUpdatesEnabled:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceBetaUpdatesEnabledChanged" object:(NSString*)appId];
}


- (NSArray*) receiverList
{
    NSMutableArray* receiverList = [NSMutableArray arrayWithCapacity:0];

    NSArray* dictList = [self getListPreference:@"ReceiverList" itemType:CFDictionaryGetTypeID()];

    for (NSDictionary* dict in dictList)
    {
        PrefReceiver* receiver = [[PrefReceiver alloc] initWithDict:dict];
        [receiverList addObject:receiver];
        [receiver release];
    }

    // return a new immutable array of receivers
    return [NSArray arrayWithArray:receiverList];
}


- (void) setReceiverList:(NSArray*)aReceiverList
{
    // construct the list in the format required for preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];

    for (uint i=0 ; i<[aReceiverList count] ; i++)
    {
        [list addObject:[[aReceiverList objectAtIndex:i] convertToDict]];
    }

    // set the preference and flush
    CFPreferencesSetAppValue(CFSTR("ReceiverList"), list, appId);
    CFPreferencesAppSynchronize(appId);
    
    // send notification that this has changed
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceReceiverListChanged"), appId, NULL, TRUE);
}


- (void) addObserverReceiverList:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceReceiverListChanged" object:(NSString*)appId];
}


- (NSArray*) selectedUdnList
{
    return [self getListPreference:@"SelectedUdnList" itemType:CFStringGetTypeID()];
}


- (void) setSelectedUdnList:(NSArray*)aSelectedUdnList
{
    // set the preference and flush
    CFPreferencesSetAppValue(CFSTR("SelectedUdnList"), aSelectedUdnList, appId);
    CFPreferencesAppSynchronize(appId);
    
    // send notification that this has changed
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceSelectedUdnListChanged"), appId, NULL, TRUE);
}


- (void) addObserverSelectedUdnList:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceSelectedUdnListChanged" object:(NSString*)appId];
}


- (NSArray*) subnetList
{
    NSMutableArray* subnetList = [NSMutableArray arrayWithCapacity:0];

    NSArray* dictList = [self getListPreference:@"SubnetList" itemType:CFDictionaryGetTypeID()];

    for (NSDictionary* dict in dictList)
    {
        PrefSubnet* subnet = [[PrefSubnet alloc] initWithDict:dict];
        [subnetList addObject:subnet];
        [subnet release];
    }

    // return a new immutable array of subnets
    return [NSArray arrayWithArray:subnetList];
}


- (void) setSubnetList:(NSArray*)aSubnetList
{
    // construct the list in the format required for preferences
    NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];

    for (uint i=0 ; i<[aSubnetList count] ; i++)
    {
        [list addObject:[[aSubnetList objectAtIndex:i] convertToDict]];
    }

    // set the preference and flush
    CFPreferencesSetAppValue(CFSTR("SubnetList"), list, appId);
    CFPreferencesAppSynchronize(appId);

    // send notification that this has changed
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceSubnetListChanged"), appId, NULL, TRUE);
}


- (void) addObserverSubnetList:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceSubnetListChanged" object:(NSString*)appId];
}


- (PrefSubnet*) selectedSubnet
{
    PrefSubnet* subnet = nil;

    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("SelectedSubnet"), appId);

    if (pref)
    {
        if (CFGetTypeID(pref) == CFDictionaryGetTypeID())
        {
            subnet = [[[PrefSubnet alloc] initWithDict:(NSDictionary*)pref] autorelease];
        }

        CFRelease(pref);
    }

    return subnet;
}


- (void) setSelectedSubnet:(PrefSubnet*)aSubnet
{
    // set the new preference value and flush
    CFPreferencesSetAppValue(CFSTR("SelectedSubnet"), [aSubnet convertToDict], appId);
    CFPreferencesAppSynchronize(appId);

    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceSelectedSubnetChanged"), appId, NULL, TRUE);
}


- (void) addObserverSelectedSubnet:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceSelectedSubnetChanged" object:(NSString*)appId];
}


- (void) refreshReceiverList
{
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("RefreshReceiverList"), appId, NULL, TRUE);
}


- (void) addObserverRefreshReceiverList:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"RefreshReceiverList" object:(NSString*)appId];
}


- (void) reconnectReceivers
{
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("ReconnectReceivers"), appId, NULL, TRUE);
}


- (void) addObserverReconnectReceivers:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"ReconnectReceivers" object:(NSString*)appId];
}


- (void) checkForUpdates
{
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("CheckForUpdates"), appId, NULL, TRUE);
}


- (void) addObserverCheckForUpdates:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"CheckForUpdates" object:(NSString*)appId];
}


- (void) synchronize
{
    CFPreferencesAppSynchronize(appId);
}


@end


