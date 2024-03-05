// CalendarWidget.h
#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QPainter>
#include "QtWidgets/qtableview.h"

class CalendarEvent;

class CalendarWidget : public QCalendarWidget {
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);

    void addEvent(const CalendarEvent &event);
    void removeEvent(const CalendarEvent &event);

    QJsonObject getJsonObject();
    void loadJsonObject(const QJsonObject &jObj);

    void saveSettings();
    void loadSettings();

    void handleContextMenuRequested(const QPoint &pos);

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

private:
    QMap<QDate, QList<CalendarEvent>> events;

    QTableView *tableView;
};

#endif // CALENDARWIDGET_H
