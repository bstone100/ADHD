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

    // different types of events
    enum Category {
        Event = 0,
        Task,
        Deadline,

        MaxOption = Deadline,
        MinOption = Event
    };
    static bool isValidCategory(Category category) {
        return category >= MinOption && category <= MaxOption;
    }
    static QString categoryToString(Category category);
    static Category stringToCategory(const QString &categoryString);

    QString description;
    Category category;

    bool allDay;

    QDateTime startDateTime;
    QDateTime endDateTime;

    QDateTime notificationDateTime;

    CalendarEvent();

    QJsonObject toJson() const;
    static CalendarEvent fromJson(const QJsonObject &obj);

    bool isValid() const {
        return isValidCategory(category) && startDateTime.isValid() && endDateTime.isValid() && notificationDateTime.isValid();
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
