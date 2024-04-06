#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

#include <QString>

#ifdef __cplusplus
extern "C" {
#endif

// Function prototype to schedule a notification, returns a QString identifier
const char* scheduleNotification(const char *title, const char *body, qint64 notificationTimeEpoch);

// Function prototype to remove a scheduled notification
void removeNotification(const char *identifier);

// Callback type for permission request result
typedef void (*PermissionRequestCallback)(bool granted);

// Requests notification permissions from the user. The result is delivered via the callback.
void requestNotificationPermission(PermissionRequestCallback callback);

#ifdef __cplusplus
}
#endif

#endif // NOTIFICATIONHELPER_H
