// AudioSessionHelper.mm
#import <AVFoundation/AVFoundation.h>
#include "AudioSessionHelper.h"

@interface AudioSessionHelper : NSObject
+ (void)deactivateAudioSession;
@end

@implementation AudioSessionHelper

+ (void)deactivateAudioSession {
    NSError *error = nil;
    [[AVAudioSession sharedInstance] setActive:NO withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:&error];
    if (error) {
        NSLog(@"Error deactivating audio session: %@", error);
    }
}

@end


void deactivateAudioSession() {
    [AudioSessionHelper deactivateAudioSession];
}
