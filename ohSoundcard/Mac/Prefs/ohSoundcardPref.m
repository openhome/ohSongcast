
#import "ohSoundcardPref.h"



@implementation ohSoundcardPref

@synthesize icon;
@synthesize textSenderName;
@synthesize buttonOnOff;
@synthesize textDescription;
@synthesize buttonShowInStatusBar;
@synthesize buttonHelp;


- (void) mainViewDidLoad
{
    // get the bundle name from the info.plist
    NSString* appName = [[[self bundle] infoDictionary] objectForKey:@"CFBundleName"];

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
        return [receiver title];
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
    }
}


@end



// Receiver class for data displayed in the table view
@implementation Receiver

@synthesize udn;
@synthesize title;
@synthesize selected;

- (id) initWithPref:(PrefReceiver*)aPref uniqueInRoom:(bool)aUnique
{
    self = [super init];

    [self setUdn:[aPref udn]];
    [self setSelected:false];

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

@end






