// HapticFeedbackHelper.mm
#import <UIKit/UIKit.h>
#include "hapticfeedback.h"

@interface HapticFeedbackHelper : NSObject
+ (void)prepareFeedback;
+ (void)generateFeedback;
@end

@implementation HapticFeedbackHelper

static UIImpactFeedbackGenerator *feedbackGenerator;

+ (void)prepareFeedback {
    if (!feedbackGenerator) {
        feedbackGenerator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
    }
    [feedbackGenerator prepare];
}

+ (void)generateFeedback {
    if (!feedbackGenerator) {
        feedbackGenerator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
    }
    [feedbackGenerator impactOccurred];
    // Optionally, dispose of feedbackGenerator or keep it for future use
//     feedbackGenerator = nil;
}

@end


void prepareHapticFeedback() {
    [HapticFeedbackHelper prepareFeedback];
}

void generateHapticFeedback() {
    [HapticFeedbackHelper generateFeedback];
}
