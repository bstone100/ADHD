#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#include "notificationhelper.h"

const char* scheduleNotification(const char *title, const char *body, qint64 notificationTimeEpoch) {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    
    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = [NSString stringWithUTF8String:title];
    content.body = [NSString stringWithUTF8String:body];
    content.sound = [UNNotificationSound defaultSound];
    
    // Calculate the time interval for the notification
    NSDate *notificationDate = [NSDate dateWithTimeIntervalSince1970:notificationTimeEpoch];
    NSDate *currentDate = [NSDate date];
    NSTimeInterval timeInterval = [notificationDate timeIntervalSinceDate:currentDate];
    
    // Ensure the time interval is in the future
    if (timeInterval < 0) {
        NSLog(@"Notification time is in the past.");
        return nullptr;
    }
    
    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:timeInterval repeats:NO];
    
    NSString *identifier = [[NSUUID UUID] UUIDString]; // Generate a unique identifier
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:trigger];
    
    [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
        if (error != nil) {
            NSLog(@"Something went wrong: %@", error);
        }
    }];
    
    // Return the identifier for later use
    return strdup([identifier UTF8String]);
}

void removeNotification(const char *identifier) {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    NSString *identifierString = [NSString stringWithUTF8String:identifier];
    [center removePendingNotificationRequestsWithIdentifiers:@[identifierString]];
}

void requestNotificationPermission(PermissionRequestCallback callback) {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                          completionHandler:^(BOOL granted, NSError * _Nullable error) {
        // Call the callback function with the result
        if (callback) {
            callback(granted);
        }
    }];
}





