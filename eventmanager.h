#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QObject>
#include "event.h"

class EventManager : public QObject
{
    Q_OBJECT
public:
    explicit EventManager(QObject *parent = nullptr);

    static EventManager *self();


    void addEvent(const Event &event);
    bool removeEvent(const QString &eventId);
    Event getEvent(const QString &eventId) const;
    bool containsEvent(const QString &eventId) const;
    void clearEvents();

private:
    QMap<QString, Event> eventMap;

    static EventManager *singleton;

signals:

};

#endif // EVENTMANAGER_H
