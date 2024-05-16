#include "sidepanel.h"
#include "../mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include "QEvent"

#if defined(Q_OS_IOS)
#include "../iOS/hapticfeedback.h"
#endif


SidePanel *SidePanel::singleton = NULL;

SidePanel::SidePanel(QWidget *parent) : QWidget(parent) {
    if (!singleton) {
        singleton = this;
    }


    closeButton = new QPushButton(MainWindow::self());
    closeButton->setStyleSheet("background: transparent; border: none; border-radius: 0px;");
    closeButton->hide();

#if defined(Q_OS_MACOS)
    connect(closeButton, &QPushButton::clicked, this, &SidePanel::collapse);
#endif

    setAttribute(Qt::WA_StyledBackground);

    vLayout = new QVBoxLayout;
    setLayout(vLayout);

    hide();

    isPanelOpen = false;

    expandAnimation = new QPropertyAnimation(this, "geometry");
    expandAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(expandAnimation, &QPropertyAnimation::finished, this, [=]{
        isPanelOpen = true;
        emit animationFinished(isPanelOpen);
    });

    collapseAnimation = new QPropertyAnimation(this, "geometry");
    collapseAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(collapseAnimation, &QPropertyAnimation::finished, this, [=]{
        isPanelOpen = false;
        emit animationFinished(isPanelOpen);
    });

#if defined(Q_OS_IOS)
    connect(expandAnimation, &QPropertyAnimation::valueChanged, this, &SidePanel::forceChildWidgetGeometry);
    connect(collapseAnimation, &QPropertyAnimation::valueChanged, this, &SidePanel::forceChildWidgetGeometry);
#endif
}

SidePanel *SidePanel::self()
{
    if (!singleton) {
        singleton = new SidePanel(MainWindow::self());
    }
    return singleton;
}

// when main window resizes
void SidePanel::updateSize()
{
    if (isPanelOpen && !isCollapsing()) {
        setGeometry(openGeometry());
        closeButton->setGeometry(closeButtonGeometry());
    }
}

bool SidePanel::event(QEvent *event)
{
    return QWidget::event(event);
}

int SidePanel::calculateWidth() const {
    return qMin(MainWindow::self()->width() * .75, 300.0);
}

QRect SidePanel::closedGeometry()
{
    int width = calculateWidth();
    return QRect(-width, 0, width, MainWindow::self()->height());
}

QRect SidePanel::openGeometry()
{
    int width = calculateWidth();
    return QRect(0, 0, width, MainWindow::self()->height());
}

QRect SidePanel::closeButtonGeometry()
{
    int width = calculateWidth();
    return QRect(width, 0, MainWindow::self()->width() - width, MainWindow::self()->height());
}

void SidePanel::touchEvent(QTouchEvent *event) {
    const QList<QTouchEvent::TouchPoint> &touchPoints = event->points();
    if (touchPoints.isEmpty()) return;

    const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();
    QPoint currentTouchPoint = touchPoint.position().toPoint();

    switch (event->type()) {
    case QEvent::TouchBegin:
        if (isHidden()) {
            setGeometry(closedGeometry());
            show();
        }

#if defined(Q_OS_IOS)
        prepareHapticFeedback();
#endif

        dx = 0;
        dt = 0;

        stopwatch.start();

        touchStartPoint = currentTouchPoint;
        previousPoint = currentTouchPoint;

        break;
    case QEvent::TouchUpdate:
    {
        dx = currentTouchPoint.x() - previousPoint.x();
        dt = stopwatch.restart();

        int newX = qBound(-calculateWidth(), x() + dx, 0); // range of x values

#if defined(Q_OS_IOS)
        int halfwayPos = calculateWidth() / 2;
        int previousPos = x() + width();
        int currentPos = newX + width();

        if (currentPos >= halfwayPos && previousPos < halfwayPos) {
            generateHapticFeedback();
            //                qDebug() << "haptic from Left: " << currentPos << halfwayPos;
        } else if (currentPos <= halfwayPos && previousPos > halfwayPos) {
            generateHapticFeedback();
            //                qDebug() << "haptic from Right: " << currentPos << halfwayPos;
        }
#endif

        move(newX, y());

#if defined(Q_OS_IOS)
        forceChildWidgetGeometry();
#endif

        previousPoint = currentTouchPoint;
    }
    break;
    case QEvent::TouchEnd:
        previousPoint = currentTouchPoint;

        handleSwipeEnd();
        break;
    default:
        break;
    }
}

void SidePanel::handleSwipeEnd() {

    if (previousPoint == touchStartPoint && closeButtonGeometry().contains(previousPoint)) {
        collapse();
        return;
    }

    float velocity = (float)dx / (float)(dt + 1); // pixels per millisecond

    int halfwayPos = calculateWidth() / 2;
    int currentPos = x() + width();

    const float thresholdVelocity = 0.3;

    if (isPanelOpen) { // decide whether to close or stay open (right to left swipe)
        if (currentPos < halfwayPos || velocity < -thresholdVelocity) {
            collapse(); // close
        } else {
            expand(); // stay open
        }
    } else { // decide whether to open or stay closed (left to right swipe)
        if (currentPos >= halfwayPos || (velocity > thresholdVelocity && currentPos > 0)) {
            expand(); // open
        } else {
            collapse(); // stay closed
        }
    }
}

void SidePanel::toggle() {
    if (isPanelOpen || isExpanding()) {
        collapse();
    } else if (!isPanelOpen || isCollapsing()) {
        expand();
    }
}

bool SidePanel::isExpanding()
{
    return expandAnimation->state() == QPropertyAnimation::Running;
}

bool SidePanel::isCollapsing()
{
    return collapseAnimation->state() == QPropertyAnimation::Running;
}

bool SidePanel::isVisibleToUser()
{
    return isVisible() && geometry().right() > 0;
}

void SidePanel::expand() {
    if (isCollapsing()) {
        collapseAnimation->stop();
    }

    if (isHidden()) {
        setGeometry(closedGeometry());
        show();
    }
    expandAnimation->setStartValue(geometry());

    expandAnimation->setEndValue(openGeometry());

    // duration proportional to distance and velo
    // v = x/t, t = x/v

    float distance = openGeometry().x() - geometry().x();
    float velocity = 0.75;
    expandAnimation->setDuration(distance / velocity); // going for around 200 ms for halfway and 400 ms for full

    closeButton->setGeometry(closeButtonGeometry());
    closeButton->show();

    expandAnimation->start();
}

void SidePanel::collapse() {

    if (isHidden()) {
        return;
    }

    if (isExpanding()) {
        expandAnimation->stop();
    }

    collapseAnimation->setStartValue(geometry());
    collapseAnimation->setEndValue(closedGeometry());

    // v = x/t, t = x/v
    float distance = geometry().x() - closedGeometry().x();
    float velocity = 0.75;
    collapseAnimation->setDuration(distance / velocity); // going for around 200 ms for halfway and 400 ms for full

    closeButton->hide();

    collapseAnimation->start();
}


void SidePanel::saveOpenChildWidgetGeometry()
{
    show();
    setGeometry(SidePanel::self()->openGeometry());

    for (int i = 0; i < vLayout->count(); i++) {
        if (QWidget* widget = vLayout->itemAt(i)->widget()) {
            openChildWidgetGeometryMap.insert(widget, widget->geometry());
        }
    }

    hide();
}

void SidePanel::forceChildWidgetGeometry()
{
    for (int i = 0; i < vLayout->count(); i++) {
        if (QWidget* widget = vLayout->itemAt(i)->widget()) {
            widget->setGeometry(openChildWidgetGeometryMap.value(widget, widget->geometry()));
        }
    }
}










