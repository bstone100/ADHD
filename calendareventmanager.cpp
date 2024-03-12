#include "calendareventmanager.h"
#include "QSettings"
#include "QJsonDocument"
#include "QJsonArray"

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
    idToEventMap.insert(event.id, event);
    dateToEventListMap[event.date].append(event);
}

void CalendarEventManager::removeEvent(const CalendarEvent &event)
{
    idToEventMap.remove(event.id);

    auto eventList = dateToEventListMap.value(event.date);
    int index = eventList.indexOf(event);
    if (index != -1) {
        eventList.removeAt(index);
        dateToEventListMap[event.date] = eventList;
    }
}

void CalendarEventManager::removeEvent(const QString &eventId) {
    removeEvent(getEvent(eventId));
}

QList<CalendarEvent> CalendarEventManager::getAllEvents()
{
    return idToEventMap.values();
}

QList<CalendarEvent> CalendarEventManager::getEventsForDate(const QDate &date)
{
    return dateToEventListMap.value(date);
}

CalendarEvent CalendarEventManager::getEvent(const QString &eventId) const {
    return idToEventMap.value(eventId);
}

bool CalendarEventManager::containsEvent(const QString &eventId) const {
    return idToEventMap.contains(eventId);
}

void CalendarEventManager::clearEvents() {
    idToEventMap.clear();
    dateToEventListMap.clear();
}

QJsonObject CalendarEventManager::getJsonObject()
{
    QJsonObject jObj;

    QJsonArray eventArray;
    foreach (auto event, idToEventMap.values()) {
        eventArray.append(event.toJson());
    }
    jObj["eventArray"] = eventArray;
    jObj["eventCount"] = eventArray.size();

    return jObj;
}

void CalendarEventManager::loadJsonObject(const QJsonObject &jObj)
{
    clearEvents();

    QJsonArray eventArray = jObj["eventArray"].toArray();

    for (int i = 0; i < eventArray.size(); i++) {
        QJsonObject eventObject = eventArray.at(i).toObject();
        CalendarEvent event = CalendarEvent::fromJson(eventObject);
        if (!event.isValid()) continue;
        addEvent(event);
    }
}

void CalendarEventManager::saveSettings()
{
    QSettings settings;
    QJsonDocument doc(getJsonObject());
    settings.setValue("calendarEvents", QString::fromUtf8(doc.toJson()));
}

void CalendarEventManager::loadSettings()
{
    QSettings settings;
    QString eventsString = settings.value("calendarEvents").toString();
    if (eventsString == "") return;
    QJsonDocument doc = QJsonDocument::fromJson(eventsString.toUtf8());
    loadJsonObject(doc.object());
}








