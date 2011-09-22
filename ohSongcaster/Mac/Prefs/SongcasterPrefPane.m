
#import "SongcasterPrefPane.h"


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
+ (NSString*) textSongcasterOff;

@end



// Implementation of preference pane
@implementation SongcasterPrefPane

@synthesize icon;
@synthesize buttonOnOff;
@synthesize textDescription;
@synthesize buttonShowInStatusBar;
@synthesize buttonHelp;


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
    
    // create the preferences object
    iPreferences = [[Preferences alloc] initWithBundle:[self bundle]];    
    [iPreferences synchronize];
    
    // get the initial receiver list
    [self preferenceReceiverListChanged:nil];

    // register for notifications from app
    [iPreferences addObserverEnabled:self selector:@selector(preferenceEnabledChanged:)];
    [iPreferences addObserverReceiverList:self selector:@selector(preferenceReceiverListChanged:)];

    // initialise UI from preferences
    [self updateButtonOnOff];
    [buttonShowInStatusBar setState:([iPreferences iconVisible] ? NSOnState : NSOffState)];
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
    NSSortDescriptor* roomSorter = [NSSortDescriptor sortDescriptorWithKey:@"room" ascending:YES];
    NSSortDescriptor* groupSorter = [NSSortDescriptor sortDescriptorWithKey:@"group" ascending:YES];
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

    [self setUdn:[aPref udn]];
    [self setSelected:false];
    [self setStatus:[aPref status]];

    if (aUnique)
    {
        [self setTitle:[aPref room]];
    }
    else
    {
        [self setTitle:[NSString stringWithFormat:@"%@ (%@)", [aPref room], [aPref group]]];
    }
    
    return self;
}


- (id) copyWithZone:(NSZone*)aZone
{
    Receiver* copy = [[Receiver alloc] init];
    [copy setUdn:[NSString stringWithString:udn]];
    [copy setTitle:[NSString stringWithString:title]];
    [copy setSelected:selected];
    [copy setStatus:status];
    return copy;    
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
static NSString* textSongcasterOff;

+ (void) loadResources:(NSBundle*)aBundle
{
    imageConnected = [[NSImage alloc] initWithContentsOfFile:[aBundle pathForResource:@"green" ofType:@"tiff"]];
    imageDisconnected = [[NSImage alloc] initWithContentsOfFile:[aBundle pathForResource:@"red" ofType:@"tiff"]];

    textConnected = NSLocalizedStringFromTableInBundle(@"TableCellStatusConnected", nil, aBundle, @"");
    textConnecting = NSLocalizedStringFromTableInBundle(@"TableCellStatusConnecting", nil, aBundle, @"");
    textDisconnected = NSLocalizedStringFromTableInBundle(@"TableCellStatusDisconnected", nil, aBundle, @"");
    textUnavailable = NSLocalizedStringFromTableInBundle(@"TableCellStatusUnavailable", nil, aBundle, @"");
    textSongcasterOff = NSLocalizedStringFromTableInBundle(@"TableCellStatusSongcasterOff", nil, aBundle, @"");
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

+ (NSString*) textSongcasterOff
{
    return textSongcasterOff;
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
        case eReceiverStateStopped:
            statusText = [CellResources textDisconnected];
            statusImage = [CellResources imageDisconnected];
            break;

        case eReceiverStateBuffering:
            statusText = [CellResources textConnecting];
            statusImage = [CellResources imageDisconnected];
            break;

        case eReceiverStatePlaying:
            statusText = [CellResources textConnected];
            statusImage = [CellResources imageConnected];
            break;
    }

    if (![[[self objectValue] objectAtIndex:1] boolValue])
    {
        statusText = [CellResources textSongcasterOff];
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








