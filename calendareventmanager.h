#ifndef CALENDARCalendarEventManager_H
#define CALENDARCalendarEventManager_H

#include <QObject>
#include "calendarevent.h"

class CalendarEventManager : public QObject
{
    Q_OBJECT
public:
    explicit CalendarEventManager(QObject *parent = nullptr);

    static CalendarEventManager *self();
    
    
    void addEvent(const CalendarEvent &event);
    bool removeEvent(const QString &eventId);
    CalendarEvent getEvent(const QString &eventId) const;
    bool containsEvent(const QString &eventId) const;
    void clearEvents();

private:
    QMap<QString, CalendarEvent> eventMap;

    static CalendarEventManager *singleton;

signals:

};

#endif // CALENDARCalendarEventManager_H
