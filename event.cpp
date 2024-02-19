#include "event.h"

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
    return Task; // Default case, or throw an exception as per your error handling policy
}

Event::Event() : id(QString::number(QUuid::createUuid().data1)), category(Task) {
}

QJsonObject Event::toJson() const {
    return QJsonObject{
        {"id", id},
        {"date", date.toString("yyyy-MM-dd ddd")},
        {"time", time.toString("HH:mm")},
        {"description", description},
        {"category", categoryToString(category)}
    };
}

Event Event::fromJson(const QJsonObject &obj) {
    Event e;
    e.id = obj["id"].toString();
    e.date = QDate::fromString(obj["date"].toString(), "yyyy-MM-dd ddd");
    e.time = QTime::fromString(obj["time"].toString(), "HH:mm");
    e.description = obj["description"].toString();
    e.category = stringToCategory(obj["category"].toString());
    return e;
}
