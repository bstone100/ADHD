// CalendarWidget.cpp
#include "calendarwidget.h"
#include "QJsonArray"
#include "QJsonObject"
#include "QSettings"
#include "QJsonDocument"
#include "../calendareventmanager.h"
#include "qmenu.h"
#include "../mainwindow.h"
#include "QHeaderView"
#include "QToolButton"
#include "svgbutton.h"

CalendarWidget::CalendarWidget(QWidget *parent) : QCalendarWidget(parent) {
    auto children = findChildren<QTableView *>();
    if (!children.isEmpty()) {
        tableView = children[0];
    }
    Q_ASSERT(tableView);

//    auto childWidgets = findChildren<QWidget *>();
//    qDebug() << childWidgets;
//    qt_calendar_prevmonth qt_calendar_nextmonth qt_calendar_monthbutton qt_calendar_yearbutton

    QToolButton *monthDropDown = findChild<QToolButton *>("qt_calendar_monthbutton");
    monthDropDown->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    monthDropDown->setCursor(Qt::PointingHandCursor);

    QToolButton *yearEditBox = findChild<QToolButton *>("qt_calendar_yearbutton");
    yearEditBox->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    yearEditBox->setCursor(Qt::PointingHandCursor);

    QAbstractButton *prevMonth = findChild<QAbstractButton *>("qt_calendar_prevmonth");
    auto layout = prevMonth->parentWidget()->layout();

    SvgButton *prevButton = new SvgButton();
    prevButton->setSvgPath(":/images/leftArrow.svg");
    prevButton->setIconSize(QSize(30,30));
    prevButton->setFixedSize(90, 50);
    prevButton->setUsingAppColors(true);

    connect(prevButton, &QPushButton::clicked, this, &QCalendarWidget::showPreviousMonth);

    QAbstractButton *nextMonth = findChild<QAbstractButton *>("qt_calendar_nextmonth");

    SvgButton *nextButton = new SvgButton();
    nextButton->setSvgPath(":/images/rightArrow.svg");
    nextButton->setIconSize(QSize(30,30));
    nextButton->setFixedSize(90, 50);
    nextButton->setUsingAppColors(true);

    connect(nextButton, &QPushButton::clicked, this, &QCalendarWidget::showNextMonth);

    layout->replaceWidget(prevMonth, prevButton);
    layout->replaceWidget(nextMonth, nextButton);

    layout->removeWidget(prevMonth);
    prevMonth->setParent(NULL);
    layout->removeWidget(nextMonth);
    nextMonth->setParent(NULL);

    QTextCharFormat format = weekdayTextFormat(Qt::Monday);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Sunday, format);

    setDateEditEnabled(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
//    setNavigationBarVisible(false);
//    setGridVisible(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &CalendarWidget::customContextMenuRequested, this, &CalendarWidget::handleContextMenuRequested);
}

void CalendarWidget::addEvent(const CalendarEvent &event) {
    events[event.date].append(event);
    updateCell(event.date);
    MainWindow::self()->saveSettings();
}

void CalendarWidget::removeEvent(const CalendarEvent &event)
{
    auto eventList = events.value(event.date);
    int index = eventList.indexOf(event);
    if (index != -1) {
        eventList.removeAt(index);
        events[event.date] = eventList;
        updateCell(event.date);
        MainWindow::self()->saveSettings();
    }
}

QJsonObject CalendarWidget::getJsonObject()
{
    QJsonObject jObj;

    QJsonArray eventArray;
    foreach (auto eventList, events) {
        foreach (auto event, eventList) {
            eventArray.append(event.toJson());
        }
    }
    jObj["eventArray"] = eventArray;
    jObj["eventCount"] = eventArray.size();

    return jObj;
}

void CalendarWidget::loadJsonObject(const QJsonObject &jObj)
{
    events.clear();
    CalendarEventManager::self()->clearEvents();

    QJsonArray eventArray = jObj["eventArray"].toArray();

    for (int i = 0; i < eventArray.size(); i++) {
        QJsonObject eventObject = eventArray.at(i).toObject();
        CalendarEvent event = CalendarEvent::fromJson(eventObject);
        if (!event.isValid()) continue;
        CalendarEventManager::self()->addEvent(event);
        events[event.date].append(event);
    }

    updateCells();
}

void CalendarWidget::saveSettings()
{
    QSettings settings;
    QJsonDocument doc(getJsonObject());
    settings.setValue("calendarEvents", QString::fromUtf8(doc.toJson()));
}

void CalendarWidget::loadSettings()
{
    QSettings settings;
    QString eventsString = settings.value("calendarEvents").toString();
    if (eventsString == "") return;
    QJsonDocument doc = QJsonDocument::fromJson(eventsString.toUtf8());
    loadJsonObject(doc.object());
}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const {
    QCalendarWidget::paintCell(painter, rect, date);

    if (events.contains(date)) {
        // Sort the events for the day by time if not already sorted
        QList<CalendarEvent> dayEvents = events.value(date);
        std::sort(dayEvents.begin(), dayEvents.end(), [](const CalendarEvent &a, const CalendarEvent &b) -> bool {
            return a.time < b.time;
        });

        // Calculate rectangle size and position
        int eventHeight = 15; // Example height for each event rectangle
        int yOffset = 0; // Starting offset for the first event

        QFont font = painter->font();
        font.setPointSize(6); // Smaller font size for event text
        painter->setFont(font);

        for (const CalendarEvent &event : dayEvents) {
            QRect eventRect = QRect(rect.left() + 2, rect.top() + 2 + yOffset, rect.width() - 4, eventHeight);
            painter->save();

            // Choose color based on the event category
            switch (event.category) {
            case Task:
                painter->setBrush(Qt::green); // Green for tasks
                break;
            case Habit:
                painter->setBrush(Qt::blue); // Blue for habits
                break;
            case Deadline:
                painter->setBrush(Qt::red); // Red for deadlines
                break;
            default:
                painter->setBrush(Qt::lightGray); // Default color
                break;
            }

            painter->drawRect(eventRect); // Draw the background rectangle for the event

            // Set text color and draw text
            painter->setPen(Qt::black); // Keeping text color black for all categories for readability
            QString eventText = QString("%1 %2 %3")
                                    .arg(event.time.toString("HH:mm"))
                                    .arg(categoryToString(event.category))
                                    .arg(event.description);

            // Draw the text inside the rectangle, consider clipping long text
            painter->drawText(eventRect.adjusted(1, 1, -1, -1), Qt::AlignLeft | Qt::AlignVCenter, eventText);

            painter->restore();

            yOffset += eventHeight + 2; // Move to the next event position
            if (yOffset + eventHeight > rect.height()) {
                // No more space for additional events, consider drawing a "+" or "more" indicator
                break; // Stop drawing more events
            }
        }
    }
}

void CalendarWidget::handleContextMenuRequested(const QPoint &pos) {
    QMenu menu;

    QAction action("Remove all events");
    connect(&action, &QAction::triggered, this, [=]{
        events.clear();
        CalendarEventManager::self()->clearEvents();
        updateCells();
    });
    menu.addAction(&action);

    menu.exec(mapToGlobal(pos));
}












