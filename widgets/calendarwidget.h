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
    QPoint centerCellPointForDate(const QDate &date);
    QRect cellRectForDate(const QDate &date);
    QSize cellSize();

    bool isDateInCurrentMonth(const QDate &date) const;

    void grabAspectRatio();


    QWidget *getCellViewWidget() const;

    QTableView *getTableView() const;

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QTableView *tableView;

    QDate pressedDate;

    double aspectRatio;
    QWidget *cellViewWidget;
};

#endif // CALENDARWIDGET_H
