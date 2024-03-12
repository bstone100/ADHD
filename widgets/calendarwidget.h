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

    void handleContextMenuRequested(const QPoint &pos);

    void updateCells();

    QDate dateAt(const QPoint &pos);
    QPoint globalPointForDate(const QDate &date) const;
    QSize cellSize();

    bool isDateInCurrentMonth(const QDate &date) const;

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QTableView *tableView;

    QDate pressedDate;
};

#endif // CALENDARWIDGET_H
