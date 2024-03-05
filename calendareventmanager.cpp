#include "calendareventmanager.h"

CalendarEventManager *CalendarEventManager::singleton = NULL;

CalendarEventManager::CalendarEventManager(QObject *parent)
    : QObject{parent}
{
    if (!singleton) {
        singleton = this;
    }
}

CalendarEventManager *CalendarEventManager::self()
{
    if (!singleton) {
        singleton = new CalendarEventManager();
    }
    return singleton;
}

void CalendarEventManager::addEvent(const CalendarEvent &event) {
    eventMap.insert(event.id, event);
}

bool CalendarEventManager::removeEvent(const QString &eventId) {
    return eventMap.remove(eventId) > 0;
}

CalendarEvent CalendarEventManager::getEvent(const QString &eventId) const {
    return eventMap.value(eventId);
}

bool CalendarEventManager::containsEvent(const QString &eventId) const {
    return eventMap.contains(eventId);
}

void CalendarEventManager::clearEvents() {
    eventMap.clear();
}



