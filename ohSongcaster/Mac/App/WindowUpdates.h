
#import <Cocoa/Cocoa.h>
#import "AutoUpdate.h"


@interface WindowUpdates : NSWindowController
{
    IBOutlet NSProgressIndicator* progressChecking;
    IBOutlet NSProgressIndicator* progressDownloading;
    IBOutlet NSView* viewChecking;
    IBOutlet NSView* viewAvailable;
    IBOutlet NSView* viewUnavailable;
    IBOutlet NSView* viewDownloading;
    AutoUpdate* iAutoUpdate;
    AutoUpdateInfo* iUpdateFound;
}

@property (assign) NSProgressIndicator* progressChecking;
@property (assign) NSProgressIndicator* progressDownloading;
@property (assign) NSView* viewChecking;
@property (assign) NSView* viewAvailable;
@property (assign) NSView* viewUnavailable;
@property (assign) NSView* viewDownloading;

- (IBAction) buttonInstallClicked:(id)aSender;
- (IBAction) buttonNotNowClicked:(id)aSender;
- (void) setAutoUpdate:(AutoUpdate*)aAutoUpdate;
- (void) startChecking;

@end
