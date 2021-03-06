
#import "SongcastPrefPane.h"


// Class for storing static resources used by table cell
@interface CellResources : NSObject
{
}

+ (void) loadResources:(NSBundle*)aBundle;
+ (NSImage*) imageConnected;
+ (NSImage*) imageDisconnected;
+ (NSString*) textConnected;
+ (NSString*) textDisconnected;
+ (NSString*) textUnavailable;
+ (NSString*) textSongcastOff;

@end



// Implementation of preference pane
@implementation SongcastPrefPane

@synthesize buttonOnOff;
@synthesize textAbout;
@synthesize textDescription;
@synthesize buttonShowInStatusBar;
@synthesize tableViewReceiverList;
@synthesize boxGettingStarted;
@synthesize boxMain;
@synthesize textStep1Text;
@synthesize buttonSongcastMode;
@synthesize textMulticastChannel;
@synthesize textLatencyMs;
@synthesize sliderLatencyMs;
@synthesize buttonNetworkAdapter;
@synthesize buttonAutoUpdates;
@synthesize buttonBeta;



- (void) mainViewDidLoad
{
    // get the bundle name from the info.plist
    NSString* appName = [[[self bundle] infoDictionary] objectForKey:@"CFBundleName"];

    [CellResources loadResources:[self bundle]];

    // initialise the text for the UI elements
    [buttonOnOff setTitle:[NSString stringWithFormat:[buttonOnOff title], appName]];
    [buttonOnOff setAlternateTitle:[NSString stringWithFormat:[buttonOnOff alternateTitle], appName]];
    [textDescription setStringValue:[NSString stringWithFormat:[textDescription stringValue], appName]];
    [buttonShowInStatusBar setTitle:[NSString stringWithFormat:[buttonShowInStatusBar title], appName]];    
    [textStep1Text setStringValue:[NSString stringWithFormat:[textStep1Text stringValue], appName]];

    // initialise the about text
    NSString* aboutFormat = [[[self bundle] infoDictionary] objectForKey:@"SongcastAboutText"];
    NSString* version = [[[self bundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    NSString* copyright = [[[self bundle] infoDictionary] objectForKey:@"NSHumanReadableCopyright"];
    [textAbout setStringValue:[NSString stringWithFormat:aboutFormat, appName, version, copyright]];
    
    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[self bundle]];    
    [iPreferences synchronize];
    
    // get the initial receiver list
    [self preferenceReceiverListChanged:nil];
    [self preferenceSubnetListChanged:nil];

    // register for notifications from app
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    [iPreferences addObserverReceiverList:self selector:@selector(preferenceReceiverListChanged:)];
    [iPreferences addObserverSubnetList:self selector:@selector(preferenceSubnetListChanged:)];

    // initialise UI from preferences
    [self updateButtonOnOff];
    [buttonShowInStatusBar setState:([iPreferences iconVisible] ? NSOnState : NSOffState)];
    [buttonSongcastMode selectCellAtRow:0 column:([iPreferences multicastEnabled] ? 1 : 0)];
    [textMulticastChannel setIntegerValue:[iPreferences multicastChannel]];
    [textLatencyMs setIntegerValue:[iPreferences latencyMs]];
    [sliderLatencyMs setIntegerValue:[iPreferences latencyMs]];
    [buttonAutoUpdates setState:([iPreferences autoUpdatesEnabled] ? NSOnState : NSOffState)];
    [buttonBeta setState:([iPreferences betaUpdatesEnabled] ? NSOnState : NSOffState)];

    // show/hide the getting started view
    if ([iPreferences hasRunWizard])
    {
        [boxMain setHidden:false];
        [boxGettingStarted setHidden:true];
    }
    else
    {
        [boxMain setHidden:true];
        [boxGettingStarted setHidden:false];
    }
}


- (void) updateButtonOnOff
{
    bool enabled = [iPreferences enabled];

    [buttonOnOff setState:(enabled ? NSOnState : NSOffState)];
}


- (IBAction) buttonOnOffClicked:(id)aSender
{
    [iPreferences setEnabled:([buttonOnOff state] == NSOnState)];
}


- (IBAction) buttonRefreshClicked:(id)aSender
{
    [iPreferences refreshReceiverList];
}


- (IBAction) buttonReconnectClicked:(id)aSender
{
    [iPreferences reconnectReceivers];
}


- (IBAction) buttonShowInStatusBarClicked:(id)aSender
{
    [iPreferences setIconVisible:([buttonShowInStatusBar state] == NSOnState)];
}


- (IBAction) buttonHelpClicked:(id)aSender
{
    NSURL* manualUrl = [NSURL URLWithString:[[[self bundle] infoDictionary] objectForKey:@"SongcastManualUrl"]];
    [[NSWorkspace sharedWorkspace] openURL:manualUrl];
}


- (IBAction) buttonWizardClicked:(id)aSender
{
    [boxGettingStarted setHidden:true];
    [boxMain setHidden:false];

    [iPreferences setHasRunWizard:true];
}


- (IBAction) buttonSongcastModeClicked:(id)aSender
{
    // get the selected radio button coordinates
    NSInteger row, column;
    [buttonSongcastMode getRow:&row column:&column ofCell:[buttonSongcastMode selectedCell]];

    // set the preference
    [iPreferences setMulticastEnabled:(column == 1)];
}


- (IBAction) buttonMulticastChannelClicked:(id)aSender
{
    // generate a new random channel - the channel is the last 2 bytes of the
    // multicast IP address 239.253.x.x
    uint32_t channel = [PrefMulticastChannel generate];

    // set preference and update UI
    [iPreferences setMulticastChannel:channel];
    [textMulticastChannel setIntegerValue:channel];
}


- (IBAction) sliderLatencyMsChanged:(id)aSender
{
    // get slider value
    uint64_t latencyMs = [sliderLatencyMs integerValue];

    // set preference and update UI
    [iPreferences setLatencyMs:latencyMs];
    [textLatencyMs setIntegerValue:latencyMs];
}


- (IBAction) buttonNetworkAdapterClicked:(id)aSender
{
    // get index of selected item
    NSInteger selected = [buttonNetworkAdapter indexOfSelectedItem];

    if (selected >= 0)
    {
        [iPreferences setSelectedSubnet:[iSubnetList objectAtIndex:selected]];
    }
}


- (IBAction) buttonAutoUpdatesClicked:(id)aSender
{
    [iPreferences setAutoUpdatesEnabled:([buttonAutoUpdates state] == NSOnState)];
}


- (IBAction) buttonBetaClicked:(id)aSender
{
    [iPreferences setBetaUpdatesEnabled:([buttonBeta state] == NSOnState)];
}


- (IBAction) buttonCheckForUpdatesClicked:(id)aSender
{
    [iPreferences checkForUpdates];
}


- (void) preferenceEnabledChanged:(NSNotification*)aNotification
{
    [iPreferences synchronize];
    [self updateButtonOnOff];
    [tableViewReceiverList reloadData];
}


- (void) preferenceReceiverListChanged:(NSNotification*)aNotification
{
    [iPreferences synchronize];

    // sort the list of receivers by room, group order
    NSSortDescriptor* roomSorter = [NSSortDescriptor sortDescriptorWithKey:@"room" ascending:YES selector:@selector(caseInsensitiveCompare:)];
    NSSortDescriptor* groupSorter = [NSSortDescriptor sortDescriptorWithKey:@"group" ascending:YES selector:@selector(caseInsensitiveCompare:)];
    NSArray* sorters = [NSArray arrayWithObjects:roomSorter, groupSorter, nil];
    NSArray* sorted = [[iPreferences receiverList] sortedArrayUsingDescriptors:sorters];

    // create the list of receiver objects to display
    NSMutableArray* receivers = [NSMutableArray arrayWithCapacity:0];

    uint i=0;
    while (i < [sorted count])
    {
        // the receiver list is sorted by room name, so receivers in the same room
        // will be adjacent in the list
        NSString* thisRoom = [[sorted objectAtIndex:i] room];

        // get the index of the next receiver that is in a different room
        uint j = i + 1;
        while (j < [sorted count] && [[[sorted objectAtIndex:j] room] compare:thisRoom] == NSOrderedSame)
        {
            j++;
        }
        
        // now create all receivers that have this room name
        bool uniqueInRoom = (j - i == 1);
        while (i < j)
        {
            Receiver* r = [[Receiver alloc] initWithPref:[sorted objectAtIndex:i] uniqueInRoom:uniqueInRoom];
            [receivers addObject:r];
            [r release];
            i++;
        }
    }

    if (iReceiverList)
    {
        [iReceiverList release];
    }
    iReceiverList = [[NSArray alloc] initWithArray:receivers];

    // update the selected flags for the data
    for (NSString* udn in [iPreferences selectedUdnList])
    {
        for (Receiver* receiver in iReceiverList)
        {
            if ([udn compare:[receiver udn]] == NSOrderedSame)
            {
                [receiver setSelected:[NSNumber numberWithBool:true]];
            }
        }
    }
    
    // refresh the table view
    [tableViewReceiverList reloadData];
}


- (void) preferenceSubnetListChanged:(NSNotification*)aNotification
{
    [iPreferences synchronize];

    // get the latest subnet list
    if (iSubnetList)
    {
        [iSubnetList release];
    }
    iSubnetList = [[iPreferences subnetList] retain];

    // get the latest selected subnet
    PrefSubnet* selected = [iPreferences selectedSubnet];

    // update UI
    [buttonNetworkAdapter removeAllItems];

    for (PrefSubnet* subnet in iSubnetList)
    {
        uint32_t address = [subnet address];
        uint32_t byte1 = address & 0xff;
        uint32_t byte2 = (address >> 8) & 0xff;
        uint32_t byte3 = (address >> 16) & 0xff;
        uint32_t byte4 = (address >> 24) & 0xff;

        NSString* title = [NSString stringWithFormat:@"%d.%d.%d.%d (%@)", byte1, byte2, byte3, byte4, [subnet name]];
        [buttonNetworkAdapter addItemWithTitle:title];

        if (selected && ([subnet address] == [selected address]))
        {
            [buttonNetworkAdapter selectItemAtIndex:[buttonNetworkAdapter numberOfItems]-1];
        }
    }

    if ([buttonNetworkAdapter numberOfItems] == 0) {
        [buttonNetworkAdapter selectItemAtIndex:-1];
    }
}


- (NSInteger) numberOfRowsInTableView:(NSTableView*)aTableView
{
    return [iReceiverList count];
}


- (id) tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aColumn row:(NSInteger)aRow
{
    NSString* checkColumn = [NSString stringWithUTF8String:"selected"];
    NSString* titleColumn = [NSString stringWithUTF8String:"title"];
    
    Receiver* receiver = [iReceiverList objectAtIndex:aRow];

    if ([titleColumn compare:[aColumn identifier]] == NSOrderedSame)
    {
        return [NSArray arrayWithObjects:receiver, [NSNumber numberWithBool:[iPreferences enabled]], nil];
    }
    
    if ([checkColumn compare:[aColumn identifier]] == NSOrderedSame)
    {
        return [NSNumber numberWithInteger:([receiver selected] ? NSOnState : NSOffState)];
    }
    
    return nil;
}


- (void) tableView:(NSTableView*)aTableView setObjectValue:(id)aValue forTableColumn:(NSTableColumn*)aColumn row:(NSInteger)aRow
{
    NSString* checkColumn = [NSString stringWithUTF8String:"selected"];

    if ([checkColumn compare:[aColumn identifier]] == NSOrderedSame)
    {
        // set selection value of the clicked row
        Receiver* receiver = [iReceiverList objectAtIndex:aRow];
        [receiver setSelected:[aValue boolValue]];
        
        // update the preferences
        NSMutableArray* list = [NSMutableArray arrayWithCapacity:0];
        for (Receiver* receiver in iReceiverList)
        {
            if ([receiver selected])
            {
                [list addObject:[receiver udn]];
            }
        }
        [iPreferences setSelectedUdnList:list];
        
        // refresh the table view
        [tableViewReceiverList reloadData];
    }
}


@end



// Receiver class for data displayed in the table view
@implementation Receiver

@synthesize udn;
@synthesize title;
@synthesize selected;
@synthesize status;


- (id) initWithPref:(PrefReceiver*)aPref uniqueInRoom:(bool)aUnique
{
    self = [super init];

    self.udn = [aPref udn];
    self.selected = false;
    self.status = [aPref status];

    if (aUnique)
    {
        self.title = [aPref room];
    }
    else
    {
        self.title = [NSString stringWithFormat:@"%@ (%@)", [aPref room], [aPref group]];
    }
    
    return self;
}


- (id) copyWithZone:(NSZone*)aZone
{
    Receiver* copy = [[Receiver alloc] init];
    [copy setUdn:udn];
    [copy setTitle:title];
    [copy setSelected:selected];
    [copy setStatus:status];
    return copy;    
}


- (void) dealloc
{
    [udn release];
    [title release];
    [super dealloc];
}

@end



// Implementation of cell resources
@implementation CellResources

static NSImage* imageConnected;
static NSImage* imageDisconnected;
static NSString* textConnected;
static NSString* textConnecting;
static NSString* textDisconnected;
static NSString* textUnavailable;
static NSString* textSongcastOff;

+ (void) loadResources:(NSBundle*)aBundle
{
    imageConnected = [[NSImage alloc] initWithContentsOfFile:[aBundle pathForResource:@"green" ofType:@"tiff"]];
    imageDisconnected = [[NSImage alloc] initWithContentsOfFile:[aBundle pathForResource:@"red" ofType:@"tiff"]];

    textConnected = @"Connected";
    textConnecting = @"Connecting";
    textDisconnected = @"Disconnected";
    textUnavailable = @"Unavailable";
    textSongcastOff = @"Songcast Off";
}

+ (NSImage*) imageConnected
{
    return imageConnected;
}

+ (NSImage*) imageDisconnected
{
    return imageDisconnected;
}

+ (NSString*) textConnected
{
    return textConnected;
}

+ (NSString*) textConnecting
{
    return textConnecting;
}

+ (NSString*) textDisconnected
{
    return textDisconnected;
}

+ (NSString*) textUnavailable
{
    return textUnavailable;
}

+ (NSString*) textSongcastOff
{
    return textSongcastOff;
}

@end



// Implementation of custom table cell
@implementation CellReceiver


- (void) drawWithFrame:(NSRect)aFrame inView:(NSView *)aView
{
    // get the data from the receiver
    Receiver* receiver = [[self objectValue] objectAtIndex:0];
    
    NSString* title = [receiver title];
    bool selected = [receiver selected];
    EReceiverState status = [receiver status];

    NSString* statusText;
    NSImage* statusImage;
    switch (status)
    {
        default:
        case eReceiverStateOffline:
            statusText = [CellResources textUnavailable];
            statusImage = [CellResources imageDisconnected];
            break;

        case eReceiverStateDisconnected:
            statusText = [CellResources textDisconnected];
            statusImage = [CellResources imageDisconnected];
            break;

        case eReceiverStateConnecting:
            statusText = [CellResources textConnecting];
            statusImage = [CellResources imageDisconnected];
            break;

        case eReceiverStateConnected:
            statusText = [CellResources textConnected];
            statusImage = [CellResources imageConnected];
            break;
    }

    if (![[[self objectValue] objectAtIndex:1] boolValue])
    {
        statusText = [CellResources textSongcastOff];
        statusImage = [CellResources imageDisconnected];
    }


    // create attribute dictionaries for the text
    NSColor* titleColor;
    NSColor* statusColor;
    
    if ([self isHighlighted])
    {
        titleColor = [NSColor alternateSelectedControlTextColor];
        statusColor = [NSColor alternateSelectedControlTextColor];
    }
    else if (selected)
    {
        titleColor = [NSColor textColor];
        statusColor = [NSColor darkGrayColor];
    }
    else
    {
        titleColor = [NSColor darkGrayColor];
        statusColor = [NSColor darkGrayColor];
    }

    NSMutableParagraphStyle* style = [[NSMutableParagraphStyle alloc] init];
    [style setParagraphStyle:[NSParagraphStyle defaultParagraphStyle]];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];

    NSDictionary* titleDict = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:13.0f], NSFontAttributeName,
                               style, NSParagraphStyleAttributeName,
                               titleColor, NSForegroundColorAttributeName,
                               nil];
    NSDictionary* statusDict = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:11.0f], NSFontAttributeName,
                                style, NSParagraphStyleAttributeName,
                                statusColor, NSForegroundColorAttributeName,
                                nil];
    [style release];


    // draw the title
    NSPoint pt = aFrame.origin;
    pt.y += (aFrame.size.height*0.5f - [title sizeWithAttributes:titleDict].height) * 0.5f;
    
    [title drawAtPoint:pt withAttributes:titleDict];

    if (selected)
    {
        // draw the status image
        NSRect rect;
        rect.size.width = 10.0f;
        rect.size.height = 10.0f;
        rect.origin = aFrame.origin;
        rect.origin.y = aFrame.origin.y + aFrame.size.height*0.5f;
        rect.origin.y += (aFrame.size.height*0.5f - rect.size.height) * 0.5f;

        [statusImage drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f];

        // draw the status text
        pt = aFrame.origin;
        pt.x += 12.0f;
        pt.y = aFrame.origin.y + aFrame.size.height*0.5f + 3.0f;

        [statusText drawAtPoint:pt withAttributes:statusDict];
    }
}


@end








