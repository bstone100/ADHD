// CalendarWidget.h
#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QPainter>
#include "QtWidgets/qtableview.h"
#include "event.h"

class CalendarWidget : public QCalendarWidget {
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);

    void addEvent(const Event &event);
    void removeEvent(const Event &event);

    QJsonObject getJsonObject();
    void loadJsonObject(const QJsonObject &jObj);

    void saveSettings();
    void loadSettings();

    void handleContextMenuRequested(const QPoint &pos);

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

private:
    QMap<QDate, QList<Event>> events;

    QTableView *tableView;
};

#endif // CALENDARWIDGET_H
