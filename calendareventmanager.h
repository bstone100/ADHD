#ifndef CALENDAREVENTMANAGER_H
#define CALENDAREVENTMANAGER_H

#include <QObject>
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

    void saveSettings();
    void loadSettings();

private:
    // these both stay updated and valid
    QMap<QString, CalendarEvent> idToEventMap;
    QMap<QDate, QList<CalendarEvent>> dateToEventListMap;

    static CalendarEventManager *singleton;

signals:

};

#endif // CALENDAREVENTMANAGER_H
