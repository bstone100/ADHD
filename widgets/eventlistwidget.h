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

class EventListWidget : public QWidget {
    Q_OBJECT

public:
    explicit EventListWidget(QWidget *parent = nullptr);
    void setDate(const QDate &date);

    void updateEvents();

    QDate getCurrentDate() const;


    QScrollArea *getScrollArea() const;

signals:
    void backButtonClicked();

private:
    QDate currentDate;
    QList<CalendarEvent> currentEvents;
    QVBoxLayout *eventsLayout;
    QLabel *dateLabel;
    QPushButton *backButton;
    QScrollArea *scrollArea;

};

#endif // EVENTLISTWIDGET_H
