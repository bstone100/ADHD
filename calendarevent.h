#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QJsonObject>
#include <QUuid>
#include <QColor>

struct CalendarEvent {
    QString id;

    QString description;

    bool allDay;

    QDateTime startDateTime;
    QDateTime endDateTime;

    QDateTime notificationDateTime;

    CalendarEvent();

    QJsonObject toJson() const;
    static CalendarEvent fromJson(const QJsonObject &obj);
    void updateFromJson(const QJsonObject &obj);

    static QDateTime parseDateTime(const QString &dateTimeStr);

    bool isValid() const {
        return startDateTime.isValid() && endDateTime.isValid() && notificationDateTime.isValid();
    }

    bool hasEndDateTime() const {
        return endDateTime > startDateTime;
    }

    bool operator==(const CalendarEvent &other) const {
        return this->id == other.id;
    }

    bool operator<(const CalendarEvent& other) const {
        if (allDay && other.allDay && startDateTime.date() == other.startDateTime.date())
            return description < other.description;

        return startDateTime < other.startDateTime;
    }
};

#endif // CALENDAREVENT_H
