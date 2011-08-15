
#import "Receiver.h"
#include "../../Soundcard.h"


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
    [pref setStatus:[self status]];
    
    return pref;
}


- (EReceiverState) status
{
    if (iPtr)
    {
        switch (ReceiverStatus(iPtr))
        {
            case eDisconnected:
                return eReceiverStateDisconnected;
            case eConnecting:
                return eReceiverStateConnecting;
            case eConnected:
                return eReceiverStateConnected;
            default:
                return eReceiverStateOffline;
        };
    }
    else
    {
        return eReceiverStateOffline;
    }
}


- (void) play
{
    if (iPtr) {
        ReceiverPlay(iPtr);
    }
}


- (void) stop
{
    if (iPtr) {
        ReceiverStop(iPtr);
    }
}


- (void) standby
{
    if (iPtr) {
        ReceiverStandby(iPtr);
    }
}


@end


