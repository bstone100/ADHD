#ifndef CALENDAREVENTMANAGER_H
#define CALENDAREVENTMANAGER_H

#include <QObject>
#include "QtCore/qjsonarray.h"
#include "calendarevent.h"

class CalendarEventManager : public QObject
{
    Q_OBJECT
public:
    explicit CalendarEventManager(QObject *parent = nullptr);

    static CalendarEventManager *self();
    
    void addEvent(const CalendarEvent &event);

    void removeEvent(const CalendarEvent &event);
    void removeEvent(const QString &eventId);

    QList<CalendarEvent> getAllEvents();
    QList<CalendarEvent> getEventsForDate(const QDate &date);

    CalendarEvent getEvent(const QString &eventId) const;
    bool containsEvent(const QString &eventId) const;
    void clearEvents();

    QJsonObject getJsonObject();
    void loadJsonObject(const QJsonObject &jObj);

    QJsonArray getAllEventsJson();
    QJsonArray getEventsForDateJson(const QDate &date);
    QJsonArray getEventsForDateRangeJson(const QDate &startDate, const QDate &endDate);

    void saveSettings();
    void loadSettings();

    void scheduleEventNotification(const CalendarEvent &event);
    void removeEventNotification(const CalendarEvent &event);

private:
    // these both stay updated and valid
    QMap<QString, CalendarEvent> idToEventMap;
    QMap<QDate, QList<CalendarEvent>> dateToEventListMap;

    QMap<QString, QString> idToNotificationMap;
    bool hasNotificationPermission;
    CalendarEvent eventToSchedule;

    QJsonArray eventListToJson(QList<CalendarEvent> events);

    static CalendarEventManager *singleton;

signals:

};

#endif // CALENDAREVENTMANAGER_H
