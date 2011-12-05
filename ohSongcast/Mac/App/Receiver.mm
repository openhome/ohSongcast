
#import "Receiver.h"
#include "../../Songcast.h"


@implementation Receiver

@synthesize udn;
@synthesize room;
@synthesize group;
@synthesize name;


- (id) initWithPtr:(void *)aPtr
{
    [super init];

    iLock = [[NSObject alloc] init];
    iPtr = aPtr;
    ReceiverAddRef(iPtr);

    self.udn = [NSString stringWithUTF8String:ReceiverUdn(iPtr)];
    self.room = [NSString stringWithUTF8String:ReceiverRoom(iPtr)];
    self.group = [NSString stringWithUTF8String:ReceiverGroup(iPtr)];
    self.name = [NSString stringWithUTF8String:ReceiverName(iPtr)];

    return self;
}


- (id) initWithPref:(PrefReceiver*)aPref
{
    [super init];

    iLock = [[NSObject alloc] init];
    iPtr = nil;

    self.udn = [aPref udn];
    self.room = [aPref room];
    self.group = [aPref group];
    self.name = [aPref name];

    return self;
}


- (void) dealloc
{
    [udn release];
    [room release];
    [group release];
    [name release];
    [super dealloc];
}


- (void) updateWithPtr:(void*)aPtr
{
    // access to iPtr must be locked
    @synchronized(iLock)
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
            self.udn = [NSString stringWithUTF8String:ReceiverUdn(iPtr)];
            self.room = [NSString stringWithUTF8String:ReceiverRoom(iPtr)];
            self.group = [NSString stringWithUTF8String:ReceiverGroup(iPtr)];
            self.name = [NSString stringWithUTF8String:ReceiverName(iPtr)];
        }
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
    // access to iPtr must be locked
    @synchronized(iLock)
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
}


- (void) play
{
    // access to iPtr must be locked
    @synchronized(iLock)
    {
        if (iPtr) {
            ReceiverPlay(iPtr);
        }
    }
}


- (void) stop
{
    // access to iPtr must be locked
    @synchronized(iLock)
    {
        if (iPtr) {
            ReceiverStop(iPtr);
        }
    }
}


- (void) standby
{
    // access to iPtr must be locked
    @synchronized(iLock)
    {
        if (iPtr) {
            ReceiverStandby(iPtr);
        }
    }
}


@end



