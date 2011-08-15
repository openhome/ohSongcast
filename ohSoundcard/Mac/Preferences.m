
#import "Preferences.h"


@implementation PrefReceiver

@synthesize udn;
@synthesize room;
@synthesize group;
@synthesize name;
@synthesize status;


- (id) initWithDict:(NSDictionary*)aDict
{
    [super init];

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



@implementation Preferences


- (id) initWithBundle:(NSBundle*)aBundle
{
    self = [super init];
    
    appId = (CFStringRef)[NSLocalizedStringFromTableInBundle(@"PreferencesAppId", @"NonLocalizable", aBundle, @"") retain];
    
    return self;
}


- (bool) iconVisible
{
    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("IconVisible"), appId);
    
    if (pref)
    {
        if (CFGetTypeID(pref) == CFBooleanGetTypeID())
        {
            CFBooleanRef visible = (CFBooleanRef)pref;
            return (visible == kCFBooleanTrue);
        }
        
        CFRelease(pref);
    }
    
    return true;
}


- (void) setIconVisible:(bool)aVisible
{
    // set the new preference value
    if (aVisible)
    {
        CFPreferencesSetAppValue(CFSTR("IconVisible"), kCFBooleanTrue, appId);
    }
    else
    {
        CFPreferencesSetAppValue(CFSTR("IconVisible"), kCFBooleanFalse, appId);
    }
    
    // flush the preferences
    CFPreferencesAppSynchronize(appId);
    
    // send notification that this has changed
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceIconVisibleChanged"), appId, NULL, TRUE);
}


- (void) addObserverIconVisible:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceIconVisibleChanged" object:(NSString*)appId];
}


- (bool) enabled
{
    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("Enabled"), appId);
    
    if (pref)
    {
        if (CFGetTypeID(pref) == CFBooleanGetTypeID())
        {
            CFBooleanRef enabled = (CFBooleanRef)pref;
            return (enabled == kCFBooleanTrue);
        }
        
        CFRelease(pref);
    }
    
    return false;
}


- (void) setEnabled:(bool)aEnabled
{
    // set the new preference value
    if (aEnabled)
    {
        CFPreferencesSetAppValue(CFSTR("Enabled"), kCFBooleanTrue, appId);
    }
    else
    {
        CFPreferencesSetAppValue(CFSTR("Enabled"), kCFBooleanFalse, appId);
    }
    
    // flush the preferences
    CFPreferencesAppSynchronize(appId);

    // send notification that this has changed
    CFNotificationCenterRef centre = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterPostNotification(centre, CFSTR("PreferenceEnabledChanged"), appId, NULL, TRUE);
}


- (void) addObserverEnabled:(id)aObserver selector:(SEL)aSelector
{
    NSDistributedNotificationCenter* centre = [NSDistributedNotificationCenter defaultCenter];
    [centre addObserver:aObserver selector:aSelector name:@"PreferenceEnabledChanged" object:(NSString*)appId];
}


- (NSArray*) receiverList
{
    // create a temporary mutable array for building the list
    NSMutableArray* receiverList = [NSMutableArray arrayWithCapacity:0];
    
    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("ReceiverList"), appId);

    if (pref)
    {
        if (CFGetTypeID(pref) == CFArrayGetTypeID())
        {
            CFArrayRef list = (CFArrayRef)pref;
        
            for (CFIndex i=0 ; i<CFArrayGetCount(list) ; i++)
            {
                const void* item = CFArrayGetValueAtIndex(list, i);
            
                if (item && CFGetTypeID(item) == CFDictionaryGetTypeID())
                {
                    PrefReceiver* receiver = [[PrefReceiver alloc] initWithDict:(NSDictionary*)item];
                    [receiverList addObject:receiver];
                    [receiver release];
                }
            }
        }

        CFRelease(pref);
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
    // create a temporary mutable array for building the list
    NSMutableArray* udnList = [NSMutableArray arrayWithCapacity:0];
    
    CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("SelectedUdnList"), appId);
    
    if (pref)
    {
        if (CFGetTypeID(pref) == CFArrayGetTypeID())
        {
            CFArrayRef list = (CFArrayRef)pref;
            
            for (CFIndex i=0 ; i<CFArrayGetCount(list) ; i++)
            {
                const void* item = CFArrayGetValueAtIndex(list, i);
                
                if (item && CFGetTypeID(item) == CFStringGetTypeID())
                {
                    NSString* udn = (NSString*)item;                    
                    [udnList addObject:udn];                    
                }
            }
        }
        
        CFRelease(pref);
    }
    
    // return a new immutable array of udns
    return [NSArray arrayWithArray:udnList];
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


- (void) synchronize
{
    CFPreferencesAppSynchronize(appId);
}


@end


