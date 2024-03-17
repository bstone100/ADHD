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
#include "sidepanel.h"
#include "svgbutton.h"

CalendarWidget::CalendarWidget(QWidget *parent) : QCalendarWidget(parent) {
    auto children = findChildren<QTableView *>();
    if (!children.isEmpty()) {
        tableView = children[0];
    }
    Q_ASSERT(tableView);

    tableView->installEventFilter(this);

//    auto children = findChildren<QWidget *>();
//    foreach (auto child, children) {
//        child->installEventFilter(this);
//    }
//    qt_scrollarea_viewport
    cellViewWidget = findChild<QWidget *>("qt_scrollarea_viewport");
    cellViewWidget->installEventFilter(this);

    eventListVisible = false;


//    auto childWidgets = findChildren<QWidget *>();
//    qDebug() << childWidgets;
//    qt_calendar_prevmonth qt_calendar_nextmonth qt_calendar_monthbutton qt_calendar_yearbutton

    monthDropDown = findChild<QToolButton *>("qt_calendar_monthbutton");
    monthDropDown->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    monthDropDown->setCursor(Qt::PointingHandCursor);
    monthDropDown->installEventFilter(this);

    yearEditBox = findChild<QToolButton *>("qt_calendar_yearbutton");
    yearEditBox->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    yearEditBox->setCursor(Qt::PointingHandCursor);
    yearEditBox->installEventFilter(this);

    QAbstractButton *prevMonth = findChild<QAbstractButton *>("qt_calendar_prevmonth");
    topLayout = prevMonth->parentWidget()->layout();

    prevButton = new SvgButton();
    prevButton->setSvgPath(":/images/leftArrow.svg");
    prevButton->setIconSize(QSize(30,30));
    prevButton->setFixedSize(90, 50);
    prevButton->setUsingAppColors(true);

    connect(prevButton, &QPushButton::clicked, this, &QCalendarWidget::showPreviousMonth);

    QAbstractButton *nextMonth = findChild<QAbstractButton *>("qt_calendar_nextmonth");

    nextButton = new SvgButton();
    nextButton->setSvgPath(":/images/rightArrow.svg");
    nextButton->setIconSize(QSize(30,30));
    nextButton->setFixedSize(90, 50);
    nextButton->setUsingAppColors(true);
    nextButton->installEventFilter(this);

    connect(nextButton, &QPushButton::clicked, this, &QCalendarWidget::showNextMonth);

    topLayout->replaceWidget(prevMonth, prevButton);
    topLayout->replaceWidget(nextMonth, nextButton);

    topLayout->removeWidget(prevMonth);
    prevMonth->setParent(NULL);
    topLayout->removeWidget(nextMonth);
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

    connect(this, &CalendarWidget::currentPageChanged, this, &CalendarWidget::cacheInitialCellGeometry);
}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const {
//    QCalendarWidget::paintCell(painter, rect, date);
//    if (MainWindow::self()->animating) {
//        qDebug() << "caught paint";
//        return;
//    }

    if (!isDateInCurrentMonth(date)) return;

    painter->setRenderHint(QPainter::Antialiasing);

    bool isToday = date == QDate::currentDate();
    bool isSelected = selectedDate() == date;
    bool isDarkMode = MainWindow::self()->isDarkModeOn();
    QColor textColor;
    QColor dotColor;
    if (isDarkMode) {
        textColor = isToday ? MainWindow::darkColor : MainWindow::lightColor;
        dotColor = isToday ? MainWindow::darkMidColor : MainWindow::lightMidColor;
    } else {
        textColor = isToday ? MainWindow::lightColor : MainWindow::darkColor;
        dotColor = isToday ? MainWindow::lightMidColor : MainWindow::darkMidColor;
    }


    // paint background for current date
    bool paintBackground = false;
    if (isToday) {
        paintBackground = true;
        if (isSelected && (MainWindow::self()->isEventListExpanding() || MainWindow::self()->isEventListCollapsing())) {
            paintBackground = false;
        }
    }
    if (paintBackground) {
        painter->save();
        painter->setBrush(isDarkMode ? MainWindow::lightColor : MainWindow::darkColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
        painter->restore();
    }

    if (CalendarEventManager::self()->getEventsForDate(date).size() > 0) {
        // Dot specifications
        int dotRadius = 4;

        // Draw the dot
        painter->save();
        painter->setBrush(dotColor); // Use a helper function to determine the color
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(rect.center(), dotRadius, dotRadius);
        painter->restore();
    }

    // Use style hint for drawing the date number
    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = QRect(rect.left(), rect.top() + (rect.height() / 12), rect.width(), rect.height());
    option.displayAlignment = Qt::AlignHCenter | Qt::AlignTop;
    option.palette.setColor(QPalette::Text, textColor);

    QString dateText = QString::number(date.day());
    this->style()->drawItemText(painter, option.rect, option.displayAlignment, option.palette, this->isEnabled(), dateText, QPalette::Text);
}

bool CalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        if (watched == cellViewWidget) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint pos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
            pressedDate = dateAt(pos);
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (watched == cellViewWidget) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint pos = mapFromGlobal(mouseEvent->globalPosition().toPoint());
            QDate date = dateAt(pos);
            if (!isDateInCurrentMonth(date) || date != pressedDate || SidePanel::self()->isVisibleToUser()) {
                event->ignore();
                return true;
            }
        }
        break;
    }
    default:
        break;
    }


    if (event->type() == QEvent::Paint) {
        if (watched == monthDropDown || watched == yearEditBox || watched == nextButton) {
            if (eventListVisible) {
                event->ignore();
                return true;
            }
        }
    }

    return QCalendarWidget::eventFilter(watched, event);
}

bool CalendarWidget::event(QEvent *event)
{
    switch (event->type()) {
//    case QEvent::Show:
    case QEvent::Resize:
        cacheInitialCellGeometry();
        break;
    default:
        break;
    }

    return QCalendarWidget::event(event);
}

QRect CalendarWidget::getTableViewInitialGeometry() const
{
    return tableViewInitialGeometry;
}

QRect CalendarWidget::getCurrentCellRectForDate(QDate date)
{
    // calculated now

    if (date.month() != monthShown() || date.year() != yearShown()) {
        // Date is not in the currently shown month/year, return center of the widget
        return QRect();
    }

    const QTableView* const view = findChild<const QTableView*>();
    Q_ASSERT(view);
    const QAbstractItemModel* const model = view->model();
    const int startCol = verticalHeaderFormat() == QCalendarWidget::NoVerticalHeader ? 0 : 1;
    const int startRow = horizontalHeaderFormat() == QCalendarWidget::NoHorizontalHeader ? 0 : 1;

    QModelIndex firstIndex;
    bool firstFound=false;
    for(int i=startRow, maxI=model->rowCount();!firstFound && i<maxI;++i){
        for(int j=startCol, maxJ=model->columnCount();!firstFound && j<maxJ;++j){
            firstIndex = model->index(i,j);
            if(firstIndex.data().toInt()==1)
                firstFound =true;
        }
    }
    const int lastDayMonth = QDate(yearShown(),monthShown(),1).addMonths(1).addDays(-1).day();
    bool lastFound=false;
    QModelIndex lastIndex;
    for(int i=model->rowCount()-1, minI=firstIndex.row();!lastFound && i>=minI;--i){
        for(int j=model->columnCount()-1;!lastFound && j>=startCol;--j){
            lastIndex= model->index(i,j);
            if(lastIndex.data().toInt()==lastDayMonth)
                lastFound=true;
        }
    }

    // Now that we have the range, find the specific cell for the date
    for (int row = firstIndex.row(); row <= lastIndex.row(); ++row) {
        for (int col = startCol; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            if (!index.isValid()) continue;

            int indexDay = index.data().toInt();
            if (indexDay == date.day()) {
                if (row == firstIndex.row() && indexDay > 7) continue;

                QRect cellRect = view->visualRect(index);
                cellRect.moveTopLeft(view->viewport()->mapToGlobal(cellRect.topLeft()));

                return cellRect;
            }
        }
    }

    // Date was not found within the month/year shown, return center of the widget
    return QRect();
}

QRect CalendarWidget::getInitialCellRectForDate(QDate date)
{
    // from map

    return QRect(dateToInitialGlobalPointMap.value(date), initialCellSize);
}

QPoint CalendarWidget::getInitialGlobalPointFromDate(QDate date)
{
    return dateToInitialGlobalPointMap.value(date);
}

QPoint CalendarWidget::getInitialLocalPointFromDate(QDate date)
{
    return dateToInitialLocalPointMap.value(date);
}

void CalendarWidget::setNavigationButtonsEnabled(bool enabled)
{
    prevButton->setEnabled(enabled);
    monthDropDown->setEnabled(enabled);
    yearEditBox->setEnabled(enabled);
    nextButton->setEnabled(enabled);
}

void CalendarWidget::handleEventListShown()
{
    eventListVisible = true;

    monthDropDown->setEnabled(false);
    yearEditBox->setEnabled(false);
    nextButton->setEnabled(false);

    disconnect(prevButton, &QPushButton::clicked, this, &QCalendarWidget::showPreviousMonth);
    connect(prevButton, &QPushButton::clicked, MainWindow::self(), &MainWindow::collapseEventList);
}

void CalendarWidget::handleEventListHidden()
{
    eventListVisible = false;

    monthDropDown->setEnabled(true);
    yearEditBox->setEnabled(true);
    nextButton->setEnabled(true);

    monthDropDown->update();
    yearEditBox->update();
    nextButton->update();

    connect(prevButton, &QPushButton::clicked, this, &QCalendarWidget::showPreviousMonth);
    disconnect(prevButton, &QPushButton::clicked, MainWindow::self(), &MainWindow::collapseEventList);
}

QTableView *CalendarWidget::getTableView() const
{
    return tableView;
}

void CalendarWidget::handleContextMenuRequested(const QPoint &pos) {
    QMenu menu;

    QAction action("Remove all events");
    connect(&action, &QAction::triggered, this, [=]{
        CalendarEventManager::self()->clearEvents();
        updateCells();
    });
    menu.addAction(&action);

    menu.exec(mapToGlobal(pos));
}

void CalendarWidget::updateCells()
{
    QCalendarWidget::updateCells();
}

QDate CalendarWidget::dateAt(const QPoint &pos)
{
    const QTableView* const view = findChild<const QTableView*>();
    Q_ASSERT(view);
    const QAbstractItemModel* const model = view->model();
    const int startCol = verticalHeaderFormat()==QCalendarWidget::NoVerticalHeader ? 0:1;
    const int startRow = horizontalHeaderFormat()==QCalendarWidget::NoHorizontalHeader ? 0:1;
    const QModelIndex clickedIndex = view->indexAt(view->viewport()->mapFromGlobal(mapToGlobal(pos)));
    if(clickedIndex.row() < startRow || clickedIndex.column() < startCol)
        return QDate();

    QModelIndex firstIndex;
    bool firstFound=false;
    for(int i=startRow, maxI=model->rowCount();!firstFound && i<maxI;++i){
        for(int j=startCol, maxJ=model->columnCount();!firstFound && j<maxJ;++j){
            firstIndex = model->index(i,j);
            if(firstIndex.data().toInt()==1)
                firstFound =true;
        }
    }
    const int lastDayMonth = QDate(yearShown(),monthShown(),1).addMonths(1).addDays(-1).day();
    bool lastFound=false;
    QModelIndex lastIndex;
    for(int i=model->rowCount()-1, minI=firstIndex.row();!lastFound && i>=minI;--i){
        for(int j=model->columnCount()-1;!lastFound && j>=startCol;--j){
            lastIndex= model->index(i,j);
            if(lastIndex.data().toInt()==lastDayMonth)
                lastFound=true;
        }
    }
    int monthShift = 0;
    int yearShift=0;
    if(clickedIndex.row()<firstIndex.row() || (clickedIndex.row()==firstIndex.row() && clickedIndex.column()<firstIndex.column())){
        if(monthShown()==1){
            yearShift=-1;
            monthShift=11;
        }
        else
            monthShift = -1;
    }
    else if(clickedIndex.row()>lastIndex.row() || (clickedIndex.row()==lastIndex.row() && clickedIndex.column()>lastIndex.column())){
        if(monthShown()==12){
            yearShift=1;
            monthShift=-11;
        }
        else
            monthShift = 1;
    }

    QDate date(yearShown()+yearShift,monthShown()+monthShift,clickedIndex.data().toInt());
    return date;
}

bool CalendarWidget::isDateInCurrentMonth(const QDate &date) const
{
    return date.month() == monthShown() && date.year() == yearShown();
}

void CalendarWidget::cacheInitialCellGeometry()
{
    const QTableView* const view = findChild<const QTableView*>();
    Q_ASSERT(view);
    const QAbstractItemModel* const model = view->model();
    const int startCol = verticalHeaderFormat() == QCalendarWidget::NoVerticalHeader ? 0 : 1;
    const int startRow = horizontalHeaderFormat() == QCalendarWidget::NoHorizontalHeader ? 0 : 1;

    QModelIndex firstIndex;
    bool firstFound=false;
    for(int i=startRow, maxI=model->rowCount();!firstFound && i<maxI;++i){
        for(int j=startCol, maxJ=model->columnCount();!firstFound && j<maxJ;++j){
            firstIndex = model->index(i,j);
            if(firstIndex.data().toInt()==1)
                firstFound =true;
        }
    }
    const int lastDayMonth = QDate(yearShown(),monthShown(),1).addMonths(1).addDays(-1).day();
    bool lastFound=false;
    QModelIndex lastIndex;
    for(int i=model->rowCount()-1, minI=firstIndex.row();!lastFound && i>=minI;--i){
        for(int j=model->columnCount()-1;!lastFound && j>=startCol;--j){
            lastIndex= model->index(i,j);
            if(lastIndex.data().toInt()==lastDayMonth)
                lastFound=true;
        }
    }


    QRect cellRect = view->visualRect(firstIndex);
    initialCellSize = cellRect.size();

    tableViewInitialGeometry = view->geometry();

    dateToInitialGlobalPointMap.clear();
    dateToInitialLocalPointMap.clear();

    // Now that we have the range, find the specific cell for the date
    for (int row = firstIndex.row(); row <= lastIndex.row(); ++row) {
        for (int col = startCol; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            if (!index.isValid()) continue;

            int indexDay = index.data().toInt();
            if (row == firstIndex.row() && indexDay > 7) continue;
            if (row == lastIndex.row() && indexDay <= 7) continue;

            QRect cellRect = view->visualRect(index);
            QPoint cellTopLeft = cellRect.topLeft();

            QDate date(yearShown(), monthShown(), indexDay);

            dateToInitialLocalPointMap.insert(date, view->viewport()->mapTo(view, cellTopLeft));
            dateToInitialGlobalPointMap.insert(date, view->viewport()->mapToGlobal(cellTopLeft));
        }
    }
}












