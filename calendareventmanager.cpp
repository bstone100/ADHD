#include "calendareventmanager.h"
#include "QSettings"
#include "QJsonDocument"
#include "QJsonArray"

#if defined(Q_OS_DARWIN)
#include "macOS/notificationhelper.h"
#endif


CalendarEventManager *CalendarEventManager::singleton = NULL;

CalendarEventManager::CalendarEventManager(QObject *parent)
    : QObject{parent}
{
    if (!singleton) {
        singleton = this;
    }

    hasNotificationPermission = false;
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

    for (QDate date = event.startDateTime.date(); date <= event.endDateTime.date(); date = date.addDays(1)) {
        dateToEventListMap[date].append(event);
    }
}

void CalendarEventManager::removeEvent(const CalendarEvent &event)
{
    idToEventMap.remove(event.id);

    for (QDate date = event.startDateTime.date(); date <= event.endDateTime.date(); date = date.addDays(1)) {
        dateToEventListMap[date].removeAll(event);
    }

    removeEventNotification(event);
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

// currently sent entirely to the LLM
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

    QJsonObject notificationObject;
    foreach (auto id, idToNotificationMap) {
        notificationObject[id] = idToNotificationMap[id];
    }
    doc.setObject(notificationObject);
    settings.setValue("notifications", QString::fromUtf8(doc.toJson()));

    settings.setValue("hasNotificationPermission", hasNotificationPermission);
}

void CalendarEventManager::loadSettings()
{
    QSettings settings;
    QString eventsString = settings.value("calendarEvents").toString();
    if (eventsString != "") {
        QJsonDocument doc = QJsonDocument::fromJson(eventsString.toUtf8());
        loadJsonObject(doc.object());
    }

    QString notiString = settings.value("notifications").toString();
    if (notiString != "") {
        QJsonObject notiObj = QJsonDocument::fromJson(notiString.toUtf8()).object();
        foreach (auto id, notiObj.keys()) {
            idToNotificationMap[id] = notiObj[id].toString();
        }
    }

    hasNotificationPermission = settings.value("hasNotificationPermission", false).toBool();
}

void CalendarEventManager::scheduleEventNotification(const CalendarEvent &event)
{
    if (!hasNotificationPermission) {
        eventToSchedule = event;
        requestNotificationPermission([](bool granted){
            CalendarEventManager::self()->hasNotificationPermission = granted;

            if (granted) {
                CalendarEventManager::self()->scheduleEventNotification(CalendarEventManager::self()->eventToSchedule);
            }
        });
        return;
    }

    qint64 notificationTimeEpoch = event.notificationDateTime.toSecsSinceEpoch();
    auto titleBytes = CalendarEvent::categoryToString(event.category).toUtf8();
    const char* title = titleBytes.constData();
    auto bodyBytes = event.description.toUtf8();
    const char* body = bodyBytes.constData();

    const char* rawIdentifier = scheduleNotification(title, body, notificationTimeEpoch);
    QString identifier = QString::fromUtf8(rawIdentifier);

    if (identifier != "") {
        qDebug() << identifier;
        idToNotificationMap.insert(event.id, identifier);
    }
}

void CalendarEventManager::removeEventNotification(const CalendarEvent &event)
{
    QString identifier = idToNotificationMap.value(event.id);
    if (identifier != "") {
        removeNotification(identifier.toUtf8().constData());
        idToNotificationMap.remove(event.id);
    }
}












