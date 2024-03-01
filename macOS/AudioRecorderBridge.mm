#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
#import <AVFoundation/AVFoundation.h>
#else
#import <AVFoundation/AVFoundation.h>
#endif
#include "AudioRecorderBridge.h"

static AVAudioRecorder* recorder = nil;

void setOutputFileName(const char* path) {
    NSString *nsPath = [NSString stringWithUTF8String:path];
    NSURL *url = [NSURL fileURLWithPath:nsPath];

    NSDictionary *settings = @{
                               AVFormatIDKey: @(kAudioFormatLinearPCM),
                               AVSampleRateKey: @44100.0,
                               AVNumberOfChannelsKey: @1,
                               AVLinearPCMBitDepthKey: @16,
                               AVLinearPCMIsNonInterleaved: @NO,
                               AVLinearPCMIsFloatKey: @NO,
                               AVLinearPCMIsBigEndianKey: @NO
                               };

    NSError *error;
    recorder = [[AVAudioRecorder alloc] initWithURL:url settings:settings error:&error];
    if (error) {
        NSLog(@"Error initializing recorder: %@", error);
        return;
    }

    recorder.meteringEnabled = YES;
}

void startRecording() {
#if TARGET_OS_IPHONE
    NSError *error = nil;

    // Configure the audio session for recording
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryRecord error:&error];
    [session setMode:AVAudioSessionModeDefault error:&error];
    [session setActive:YES withOptions:0 error:&error];

    if (error) {
        NSLog(@"Error configuring audio session: %@", error);
        return;
    }
#endif

    if (!recorder.isRecording) {
        [recorder prepareToRecord];
        BOOL started = [recorder record];
        if (!started) {
            NSLog(@"Failed to start recording");
        }
    }
}

void stopRecording() {
#if TARGET_OS_IPHONE
    [[AVAudioSession sharedInstance] setActive:NO error:nil];
#endif
    if (recorder.isRecording) {
        [recorder stop];
    }
}

float getCurrentLevel() {
    [recorder updateMeters];
    float averagePower = [recorder averagePowerForChannel:0];
    float linearLevel = pow(10, averagePower / 20); // Convert dB to linear
    return linearLevel;
}

bool isRecording() {
    return recorder.isRecording ? true : false;
}
