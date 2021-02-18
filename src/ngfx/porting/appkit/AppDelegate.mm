#include "porting/appkit/AppDelegate.h"

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSWindow* mainWindow = [[NSApplication sharedApplication] mainWindow];
    [mainWindow makeKeyAndOrderFront:self];
}
- (void)applicationWillTerminate:(NSNotification *)aNotification {}
-(BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
    return YES;
}
@end
