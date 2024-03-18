// CalendarWidget.h
#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QPainter>
#include "QtWidgets/qtableview.h"
#include "QToolButton"

class CalendarEvent;
class SvgButton;

class CalendarWidget : public QCalendarWidget {
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);

    void handleContextMenuRequested(const QPoint &pos);

    void updateCells();

    QDate dateAt(const QPoint &pos);

    bool isDateInCurrentMonth(const QDate &date) const;

    void cacheInitialCellGeometry();

    QTableView *getTableView() const;
    QRect getTableViewInitialGeometry() const;

    // in global space
    QRect getCurrentCellRectForDate(QDate date);
    QRect getInitialCellRectForDate(QDate date);

    QPoint getInitialGlobalPointFromDate(QDate date);
    QPoint getInitialLocalPointFromDate(QDate date);

    void setNavigationButtonsEnabled(bool enabled);
    void handleEventListShown();
    void handleEventListHidden();

    QWidget *getCellViewWidget() const;

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;

private:
    QTableView *tableView;

    QLayout *topLayout;
    QToolButton *monthDropDown;
    QToolButton *yearEditBox;
    SvgButton *prevButton;
    SvgButton *nextButton;

    bool eventListVisible;

    QDate pressedDate;

    // registers clicks on top of the table view
    QWidget *cellViewWidget;

    QRect tableViewInitialGeometry;
    QSize initialCellSize;
    QMap<QDate, QPoint> dateToInitialGlobalPointMap;
    QMap<QDate, QPoint> dateToInitialLocalPointMap;
};

#endif // CALENDARWIDGET_H
