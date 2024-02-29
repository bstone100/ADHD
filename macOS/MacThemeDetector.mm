#import <AppKit/AppKit.h>
#include "MacThemeDetector.h"

bool isMacInDarkMode() {
    NSString *osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return [osxMode isEqualToString:@"Dark"];
}
