
#import "ReceiverList.h"
#include "../../Soundcard.h"



// Definition for a class used to pass arguments across
// the soundcard-main thread boundary
@interface CallbackArg : NSObject
{
    void* ptr;
}

@property (readonly) void* ptr;

- (id) initWithPtr:(void*)aPtr;

@end



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


- (void) receiverAdded:(CallbackArg*)aArg
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn([aArg ptr])];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    // update existing receivers and add new ones
    if (receiver)
    {
        [receiver updateWithPtr:[aArg ptr]];
    }
    else
    {
        receiver = [[[Receiver alloc] initWithPtr:[aArg ptr]] autorelease];
        [iList addObject:receiver];
    }
    
    // notify of changes
    [iObserver receiverListChanged];
    
    // the ref count of the passed in arg can now be decremented - this is to match
    // the add ref call in the C-style callback method that is called which dispatches
    // this method call to the main thread (ReceiverListCallback, below)
    ReceiverRemoveRef([aArg ptr]);
}


- (void) receiverRemoved:(CallbackArg*)aArg
{
    // look for this receiver in the current list of receivers
    NSString* udn = [NSString stringWithUTF8String:ReceiverUdn([aArg ptr])];
    
    Receiver* receiver = [self receiverWithUdn:udn];
    
    if (receiver)
    {
        // receiver is in the list - clear the pointer
        [receiver updateWithPtr:nil];
        
        // notify of changes
        [iObserver receiverListChanged];
    }
    
    // the ref count of the passed in arg can now be decremented - this is to match
    // the add ref call in the C-style callback method that is called which dispatches
    // this method call to the main thread (ReceiverListCallback, below)
    ReceiverRemoveRef([aArg ptr]);
}


- (void) receiverChanged:(CallbackArg*)aArg
{
    [self receiverAdded:aArg];
}


@end



// Implementation of the callback arg class
@implementation CallbackArg

@synthesize ptr;

- (id) initWithPtr:(void*)aPtr
{
    self = [super init];
    ptr = aPtr;
    return self;
}

@end



// Callback from the ohSoundcard code for a receiver event
void ReceiverListCallback(void* aPtr, ECallbackType aType, THandle aReceiver)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    ReceiverList* receiverList = (ReceiverList*)aPtr;

    // The receiver is passed asynchronously to the main thread - add a ref that gets decremented
    // in the main thread functions
    ReceiverAddRef(aReceiver);

    switch (aType)
    {
        case eAdded:
            [receiverList performSelectorOnMainThread:@selector(receiverAdded:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
        case eRemoved:
            [receiverList performSelectorOnMainThread:@selector(receiverRemoved:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
        case eChanged:
            [receiverList performSelectorOnMainThread:@selector(receiverChanged:) withObject:[[[CallbackArg alloc] initWithPtr:aReceiver] autorelease] waitUntilDone:FALSE];
            break;
    }
    
    [pool drain];
}


