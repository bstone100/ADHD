#ifndef EVENTLISTWIDGET_H
#define EVENTLISTWIDGET_H

#include <QWidget>
#include <QDate>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QList>
#include <QTime>
#include "../calendarevent.h"

class SvgButton;

class EventListWidget : public QWidget {
    Q_OBJECT

public:
    explicit EventListWidget(QWidget *parent = nullptr);
    void setDate(const QDate &date);

    void updateEvents();

    QDate getCurrentDate() const;

    QScrollArea *getScrollArea() const;

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QDate currentDate;
    QList<CalendarEvent> currentEvents;
    QVBoxLayout *eventsLayout;
    QLabel *dateLabel;
    QScrollArea *scrollArea;

};

#endif // EVENTLISTWIDGET_H
