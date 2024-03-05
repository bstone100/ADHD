#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QJsonObject>
#include <QUuid>

enum Category {
    Task,
    Habit,
    Deadline
};

QString categoryToString(Category category);
Category stringToCategory(const QString &categoryString);

struct CalendarEvent {
    QString id;
    QDate date;
    QTime time;
    QString description;
    Category category;

    CalendarEvent();

    QJsonObject toJson() const;
    static CalendarEvent fromJson(const QJsonObject &obj);

    bool isValid() const {
        return date.isValid() && time.isValid();
    }

    bool operator==(const CalendarEvent &other) const {
        return this->id == other.id;
    }

    bool operator<(const CalendarEvent& other) const {
        if (this->date < other.date) {
            return true;
        } else if (this->date > other.date) {
            return false;
        } else {
            // Dates are equal, compare times
            return this->time < other.time;
        }
    }
};

#endif // CALENDAREVENT_H
