
#import <Foundation/Foundation.h>



@interface AutoUpdateInfo : NSObject
{
    NSString* version;
    NSString* uri;
}

@property (retain) NSString* version;
@property (retain) NSString* uri;

@end



@interface AutoUpdate : NSObject
{
    NSString* iFeedUri;
    NSString* iAppName;
    NSString* iTarget;
    NSString* iCurrentVersion;
    NSString* iRelativeDataPath;
    bool iCheckForBeta;
}

- (id) initWithFeedUri:(NSString*)aFeedUri appName:(NSString*)aAppName currentVersion:(NSString*)aCurrentVersion relativeDataPath:(NSString*)aRelativeDataPath;
- (void) setCheckForBeta:(bool)aCheckForBeta;
- (AutoUpdateInfo*) checkForUpdates;
- (bool) installUpdate:(AutoUpdateInfo*)aInfo;

@end



