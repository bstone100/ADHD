#include "eventmanager.h"

EventManager *EventManager::singleton = NULL;

EventManager::EventManager(QObject *parent)
    : QObject{parent}
{
    if (!singleton) {
        singleton = this;
    }
}

EventManager *EventManager::self()
{
    if (!singleton) {
        singleton = new EventManager();
    }
    return singleton;
}

void EventManager::addEvent(const Event &event) {
    eventMap.insert(event.id, event);
}

bool EventManager::removeEvent(const QString &eventId) {
    return eventMap.remove(eventId) > 0;
}

Event EventManager::getEvent(const QString &eventId) const {
    return eventMap.value(eventId);
}

bool EventManager::containsEvent(const QString &eventId) const {
    return eventMap.contains(eventId);
}

void EventManager::clearEvents() {
    eventMap.clear();
}



