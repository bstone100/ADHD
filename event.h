#ifndef EVENT_H
#define EVENT_H

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

struct Event {
    QString id;
    QDate date;
    QTime time;
    QString description;
    Category category;

    Event();

    QJsonObject toJson() const;
    static Event fromJson(const QJsonObject &obj);

    bool isValid() const {
        return date.isValid() && time.isValid();
    }

    bool operator==(const Event &other) const {
        return this->id == other.id;
    }

    bool operator<(const Event& other) const {
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

#endif // EVENT_H
