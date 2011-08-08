
#import "ReceiverList.h"
#include "../../Soundcard.h"



// Implementation of receiver class
@implementation Receiver

@synthesize udn;
@synthesize room;
@synthesize group;
@synthesize name;


- (id) initWithPtr:(void *)aPtr
{
    [super init];
    
    iPtr = aPtr;
    ReceiverAddRef(iPtr);
    
    udn = [[NSString alloc] initWithUTF8String:ReceiverUdn(iPtr)];
    room = [[NSString alloc] initWithUTF8String:ReceiverRoom(iPtr)];
    group = [[NSString alloc] initWithUTF8String:ReceiverGroup(iPtr)];
    name = [[NSString alloc] initWithUTF8String:ReceiverName(iPtr)];    
    
    return self;
}


- (id) initWithPref:(PrefReceiver*)aPref
{
    [super init];
    
    iPtr = nil;
    
    udn = [[aPref udn] retain];
    room = [[aPref room] retain];
    group = [[aPref group] retain];
    name = [[aPref name] retain];
    
    return self;
}


- (void) updateWithPtr:(void*)aPtr
{
    if (iPtr != aPtr)
    {
        if (iPtr)
            ReceiverRemoveRef(iPtr);
        if (aPtr)
            ReceiverAddRef(aPtr);
    }
    
    iPtr = aPtr;
    
    if (aPtr)
    {
        [udn release];
        [room release];
        [group release];
        [name release];
        udn = [[NSString alloc] initWithUTF8String:ReceiverUdn(iPtr)];
        room = [[NSString alloc] initWithUTF8String:ReceiverRoom(iPtr)];
        group = [[NSString alloc] initWithUTF8String:ReceiverGroup(iPtr)];
        name = [[NSString alloc] initWithUTF8String:ReceiverName(iPtr)];
    }
}


- (PrefReceiver*) convertToPref
{
    PrefReceiver* pref = [[[PrefReceiver alloc] init] autorelease];
    
    [pref setUdn:udn];
    [pref setRoom:room];
    [pref setGroup:group];
    [pref setName:name];
    [pref setAvailable:(iPtr != nil)];
    
    return pref;
}


@end



// Implementation of receiver class
@implementation ReceiverList


- (id) initWithReceivers:(NSArray*)aReceivers
{
    self = [super init];
    
    iList = [[NSMutableArray alloc] initWithArray:aReceivers];
    iObserver = nil;
    
    return self;
}


- (NSArray*) receivers
{
    return iList;
}


- (void) addObserver:(id<IReceiverListObserver>)aObserver
{
    iObserver = aObserver;
}


- (Receiver*) receiverWithUdn:(NSString*)aUdn
{
    for (Receiver* receiver in iList)
    {
        if ([[receiver udn] compare:aUdn] == NSOrderedSame)
        {
            return receiver;
        }
    }
    
    return nil;
}


- (void) receiverAdded:(void*)aReceiver
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn(aReceiver)];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    // update existing receivers and add new ones
    if (receiver)
    {
        [receiver updateWithPtr:aReceiver];
    }
    else
    {
        receiver = [[[Receiver alloc] initWithPtr:aReceiver] autorelease];
        [iList addObject:receiver];
    }
    
    // notify of changes
    [iObserver receiverListChanged];
}


- (void) receiverRemoved:(void*)aReceiver
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn(aReceiver)];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    if (receiver)
    {
        // receiver is in the list - clear the pointer
        [receiver updateWithPtr:nil];
        
        // notify of changes
        [iObserver receiverListChanged];
    }
}


- (void) receiverChanged:(void*)aReceiver
{
    [self receiverAdded:aReceiver];
}


@end



// Callback from the ohSoundcard code for a receiver event
void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    ReceiverList* receiverList = (ReceiverList*)aPtr;
    switch (aType)
    {
        case eAdded:
            [receiverList receiverAdded:aReceiver];
            break;
        case eRemoved:
            [receiverList receiverRemoved:aReceiver];
            break;
        case eChanged:
            [receiverList receiverChanged:aReceiver];
            break;
    }
    
    [pool drain];
}


