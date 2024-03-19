#include "calendarevent.h"

QString categoryToString(Category category) {
    switch (category) {
    case Task: return "Task";
    case Habit: return "Habit";
    case Deadline: return "Deadline";
    default: return "Unknown";
    }
}

Category stringToCategory(const QString &categoryString) {
    if (categoryString == "Task") return Task;
    if (categoryString == "Habit") return Habit;
    if (categoryString == "Deadline") return Deadline;
    return Task;
}

QColor colorForCategory(Category category) {
    switch (category) {
    case Task:
        return Qt::green;
    case Habit:
        return Qt::blue;
    case Deadline:
        return Qt::red;
    default:
        return Qt::lightGray;
    }
}

CalendarEvent::CalendarEvent() : id(QString::number(QUuid::createUuid().data1)), category(Task) {
}

QJsonObject CalendarEvent::toJson() const {
    return QJsonObject{
        {"id", id},
        {"date", date.toString("yyyy-MM-dd ddd")},
        {"time", time.toString("HH:mm")},
        {"description", description},
        {"category", categoryToString(category)}
    };
}

CalendarEvent CalendarEvent::fromJson(const QJsonObject &obj) {
    CalendarEvent e;
    e.id = obj["id"].toString();
    e.date = QDate::fromString(obj["date"].toString(), "yyyy-MM-dd ddd");
    e.time = QTime::fromString(obj["time"].toString(), "HH:mm");
    e.description = obj["description"].toString();
    e.category = stringToCategory(obj["category"].toString());
    return e;
}
