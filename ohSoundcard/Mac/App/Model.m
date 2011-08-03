
#import "Model.h"


@implementation Model


- (id)init
{
    iEnabled = FALSE;

    return self;
}


- (void)start
{
}


- (void)stop
{
}


- (bool)enabled
{
    return iEnabled;
}


- (void)setEnabled:(bool)aValue
{
    iEnabled = aValue;
}


@end
