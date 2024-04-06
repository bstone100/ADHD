#include "calendarevent.h"


QString CalendarEvent::categoryToString(Category category)
{
    switch (category) {
    case Event: return "Event";
    case Task: return "Task";
    case Deadline: return "Deadline";
    default: return "Unknown";
    }
}

CalendarEvent::Category CalendarEvent::stringToCategory(const QString &categoryString)
{
    if (categoryString == "Event") return Event;
    if (categoryString == "Task") return Task;
    if (categoryString == "Deadline") return Deadline;
    return (Category)-1;
}

CalendarEvent::CalendarEvent()
{
    id = QString::number(QUuid::createUuid().data1);
    category = (Category)-1;
}

QJsonObject CalendarEvent::toJson() const {
    return QJsonObject{
        {"id", id},

        {"description", description},
        {"category", categoryToString(category)},

        {"allDay", allDay},
        {"startDateTime", startDateTime.toString("yyyy-MM-dd ddd HH:mm")},
        {"endDateTime", endDateTime.toString("yyyy-MM-dd ddd HH:mm")},
        {"notificationDateTime", notificationDateTime.toString("yyyy-MM-dd ddd HH:mm")}
    };
}

CalendarEvent CalendarEvent::fromJson(const QJsonObject &obj) {
    CalendarEvent e;
    e.id = obj["id"].toString(e.id);
    e.description = obj["description"].toString();
    e.category = stringToCategory(obj["category"].toString());
    e.allDay = obj["allDay"].toBool(false);

    auto parseDateTime = [&](const QString &dateTimeStr) -> QDateTime {
        QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd ddd HH:mm");
        if (!dateTime.isValid()) {
            QDate date = QDate::fromString(dateTimeStr, "yyyy-MM-dd ddd");
            if (date.isValid()) {
                dateTime = QDateTime(date, QTime());
            }
        }
        return dateTime;
    };

    switch (e.category) {
    case Event:
        e.startDateTime = parseDateTime(obj["startDateTime"].toString());
        e.endDateTime = parseDateTime(obj["endDateTime"].toString());
        if (!e.endDateTime.isValid()) {
            e.allDay = true;
            e.endDateTime = e.startDateTime;
        }
        break;
    case Deadline:
    case Task:
        e.startDateTime = parseDateTime(obj["startDateTime"].toString());
        e.endDateTime = e.startDateTime;
        break;
    }

    e.notificationDateTime = QDateTime::fromString(obj["notificationDateTime"].toString(), "yyyy-MM-dd ddd HH:mm");
    if (!e.notificationDateTime.isValid()) {
        e.notificationDateTime = e.startDateTime;
    }

    return e;
}






