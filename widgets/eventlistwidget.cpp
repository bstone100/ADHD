#include "eventlistwidget.h"
#include <QHBoxLayout>
#include <QDateTime>
#include "../calendareventmanager.h"
#include "QScroller"

EventListWidget::EventListWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    setAttribute(Qt::WA_StyledBackground);

    // Layout for centering the date label
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateLabel = new QLabel(this);
    dateLabel->setStyleSheet("font-size: 20px;");
    dateLabel->installEventFilter(this);
    dateLabel->setAlignment(Qt::AlignCenter); // Ensure text is centered in the label
    dateLayout->addWidget(dateLabel, 0, Qt::AlignCenter); // Add the dateLabel to the layout with centered alignment

    scrollArea = new QScrollArea(this);
    scrollAreaContent = new QWidget();
    scrollAreaContent->setStyleSheet("background: transparent;");

    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    eventsLayout = new QVBoxLayout(scrollAreaContent);
    scrollAreaContent->setLayout(eventsLayout);

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    QScroller* scroller = QScroller::scroller(scrollArea);

    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);

    scroller->setScrollerProperties(properties);
    scroller->grabGesture(scrollArea, QScroller::TouchGesture);

    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif

    mainLayout->addLayout(dateLayout); // Add date layout below the top layout
    mainLayout->addWidget(scrollArea);
}

void EventListWidget::setDate(const QDate &date) {
    currentDate = date;
    dateLabel->setText(currentDate.toString("dddd, MMMM d, yyyy"));
    updateEvents();
}

void EventListWidget::updateEvents() {
    // Clear the current layout
    QLayoutItem *child;
    while ((child = eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget(); // delete the widget
        delete child; // delete the layout item
    }

    auto events = CalendarEventManager::self()->getEventsForDate(currentDate);

    if (events.isEmpty()) {
        // If there are no events, display a message to the user
        QLabel *noEventsLabel = new QLabel("No events for this date", this);
        noEventsLabel->setAlignment(Qt::AlignCenter);
        eventsLayout->addWidget(noEventsLabel, 0, Qt::AlignCenter);
    } else {
        auto events = CalendarEventManager::self()->getEventsForDate(currentDate);
        events.removeIf([](const CalendarEvent& event) { return !event.isValid(); });
        std::sort(events.begin(), events.end());

        auto allDayEvents = events;
        allDayEvents.removeIf([](const CalendarEvent& event) { return !event.allDay; });

        auto startTimeEvents = events;
        startTimeEvents.removeIf([](const CalendarEvent& event) { return event.allDay; });

        foreach (auto event, allDayEvents) {
            QString label;

            switch (event.category) {
            case CalendarEvent::Event:
            case CalendarEvent::Task:
                label = QString("All day:  %1")
                            .arg(event.description);
                break;
            case CalendarEvent::Deadline:
                label = QString("!!! All day:  %1")
                            .arg(event.description);
                break;
            default:
                break;
            }

            QLabel *eventLabel = new QLabel(label, this);
            eventsLayout->addWidget(eventLabel);
        }

        if (!allDayEvents.isEmpty()) {
            eventsLayout->addSpacing(20);
        }

        foreach (auto event, startTimeEvents) {
            QString label;

            switch (event.category) {
            case CalendarEvent::Event:
                label = QString("%1 - %2: %3")
                            .arg(event.startDateTime.toString("h:mm A"), event.endDateTime.toString("h:mm A"), event.description);
                break;
            case CalendarEvent::Task:
                label = QString("%1: %2")
                            .arg(event.startDateTime.toString("h:mm A"), event.description);
                break;
            case CalendarEvent::Deadline:
                label = QString("!!! %1: %2")
                            .arg(event.startDateTime.toString("h:mm A"), event.description);
                break;
            default:
                break;
            }

            QLabel *eventLabel = new QLabel(label, this);
            eventsLayout->addWidget(eventLabel);
        }

        eventsLayout->addStretch(1);
    }
}

QDate EventListWidget::getCurrentDate() const
{
    return currentDate;
}

QScrollArea *EventListWidget::getScrollArea() const
{
    return scrollArea;
}

bool EventListWidget::event(QEvent *event)
{
    return QWidget::event(event);
}

bool EventListWidget::eventFilter(QObject *watched, QEvent *event)
{
    return QWidget::eventFilter(watched, event);
}

QWidget *EventListWidget::getScrollAreaContent() const
{
    return scrollAreaContent;
}






