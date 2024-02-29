// DarkModeDetector.mm
#import <UIKit/UIKit.h>
#include "DarkModeDetector.h"

@interface DarkModeHelper : NSObject
+ (BOOL)checkSystemDarkMode;
@end

@implementation DarkModeHelper

+ (BOOL)checkSystemDarkMode {
    if (@available(iOS 12.0, *)) {
        UIUserInterfaceStyle style = UIScreen.mainScreen.traitCollection.userInterfaceStyle;
        return style == UIUserInterfaceStyleDark;
    } else {
        // Fallback for iOS versions that do not support dark mode
        return NO;
    }
}

@end

bool isIOSInDarkMode() {
    return [DarkModeHelper checkSystemDarkMode];
}
