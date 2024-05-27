#include "calendarevent.h"


CalendarEvent::CalendarEvent()
{
    id = QString::number(QUuid::createUuid().data1);
}

QJsonObject CalendarEvent::toJson() const {
    return QJsonObject{
        {"id", id},
        {"description", description},
        {"allDay", allDay},
        {"startDateTime", startDateTime.toString("yyyy-MM-dd HH:mm")},
        {"endDateTime", endDateTime.toString("yyyy-MM-dd HH:mm")},
        {"notificationDateTime", notificationDateTime.toString("yyyy-MM-dd HH:mm")}
    };
}

CalendarEvent CalendarEvent::fromJson(const QJsonObject &obj) {
    CalendarEvent e;
    e.id = obj["id"].toString(e.id);
    e.description = obj["description"].toString();
    e.allDay = obj["allDay"].toBool(false);

    e.startDateTime = parseDateTime(obj["startDateTime"].toString());
    e.endDateTime = parseDateTime(obj["endDateTime"].toString());

    if (!e.endDateTime.isValid() && e.startDateTime.isValid()) {
        e.endDateTime = e.startDateTime;
    }
    if (!e.startDateTime.isValid() && e.endDateTime.isValid()) {
        e.startDateTime = e.endDateTime;
    }
    if (e.endDateTime < e.startDateTime) {
        e.endDateTime = e.startDateTime;
    }

    e.notificationDateTime = QDateTime::fromString(obj["notificationDateTime"].toString(), "yyyy-MM-dd HH:mm");
    if (!e.notificationDateTime.isValid()) {
        e.notificationDateTime = e.startDateTime;
    }

    return e;
}

void CalendarEvent::updateFromJson(const QJsonObject &obj)
{
    if (obj.contains("description")) {
        description = obj["description"].toString();
    }

    if (obj.contains("allDay")) {
        allDay = obj["allDay"].toBool(false);
    }

    qint64 notiDelta = startDateTime.secsTo(notificationDateTime);

    if (obj.contains("startDateTime")) {
        QDateTime newStartDateTime = parseDateTime(obj["startDateTime"].toString());
        if (newStartDateTime.isValid()) {
            startDateTime = newStartDateTime;
            notificationDateTime = startDateTime.addSecs(notiDelta); // auto update noti time
        }
    }

    if (obj.contains("endDateTime")) {
        QDateTime newEndDateTime = parseDateTime(obj["endDateTime"].toString());
        if (newEndDateTime.isValid()) {
            endDateTime = newEndDateTime;
        }
    }

    if (!endDateTime.isValid() && startDateTime.isValid()) {
        endDateTime = startDateTime;
    }
    if (!startDateTime.isValid() && endDateTime.isValid()) {
        startDateTime = endDateTime;
    }
    if (endDateTime < startDateTime) {
        endDateTime = startDateTime;
    }

    if (obj.contains("notificationDateTime")) {
        QDateTime newNotificationDateTime = QDateTime::fromString(obj["notificationDateTime"].toString(), "yyyy-MM-dd HH:mm");
        if (newNotificationDateTime.isValid()) {
            notificationDateTime = newNotificationDateTime;
        }
    }
}

QDateTime CalendarEvent::parseDateTime(const QString &dateTimeStr)
{
    QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd HH:mm");
    if (!dateTime.isValid()) {
        QDate date = QDate::fromString(dateTimeStr, "yyyy-MM-dd ddd");
        if (date.isValid()) {
            dateTime = QDateTime(date, QTime());
        }
    }
    return dateTime;
}






