#include "SidePanel.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include "QEvent"

SidePanel *SidePanel::singleton = NULL;

SidePanel::SidePanel(QWidget *parent) : QWidget(parent) {
    if (!singleton) {
        singleton = this;
    }

    closeButton = new QPushButton(MainWindow::self());
    closeButton->setStyleSheet("background: transparent; border: none; border-radius: 0px;");
    closeButton->hide();

    connect(closeButton, &QPushButton::clicked, this, &SidePanel::collapse);

    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("SidePanel{background-color: #d8c7f0;}");

    vLayout = new QVBoxLayout;
    setLayout(vLayout);

    hide();
}

SidePanel *SidePanel::self()
{
    if (!singleton) {
        singleton = new SidePanel(MainWindow::self());
    }
    return singleton;
}

void SidePanel::expand()
{
    if (isHidden()) {
        toggle();
    }
}

void SidePanel::collapse()
{
    if (isVisible()) {
        toggle();
    }
}

void SidePanel::toggle() {

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200); // Animation duration in milliseconds
    animation->setEasingCurve(QEasingCurve::InOutCubic);

    int width = calculateWidth();
    QRect closedGeometry(-width, 0, width, MainWindow::self()->height());
    QRect openGeometry(0, 0, width, MainWindow::self()->height());

    bool aboutToOpen = isHidden();

    QRect endRect;
    if (aboutToOpen) {
        setGeometry(closedGeometry);
        show();
        endRect = openGeometry;

        QRect closeButtonGeometry(width, 0, MainWindow::self()->width() - width, MainWindow::self()->height());
        closeButton->setGeometry(closeButtonGeometry);
        closeButton->show();
    } else {
        connect(animation, &QPropertyAnimation::finished, this, &QWidget::hide);
        endRect = closedGeometry;

        closeButton->hide();
    }

    connect(animation, &QPropertyAnimation::finished, this, [=]{emit animationFinished(aboutToOpen);});

    animation->setStartValue(geometry());
    animation->setEndValue(endRect);

// prevent flickering temporary fix
#if defined(Q_OS_IOS)
    for (int i = 0; i < vLayout->count(); ++i) {
        if (QWidget* widget = vLayout->itemAt(i)->widget()) {
            widget->setUpdatesEnabled(false);
        }
    }
    connect(animation, &QPropertyAnimation::finished, this, [this]{
        for (int i = 0; i < vLayout->count(); ++i) {
            if (QWidget* widget = vLayout->itemAt(i)->widget()) {
                widget->setUpdatesEnabled(true);
            }
        }
    });
#endif

    animation->start(QPropertyAnimation::DeleteWhenStopped);

    emit animationStarted(aboutToOpen);
}

// when main window resizes
void SidePanel::updateSize()
{
    int width = calculateWidth();
    QRect openGeometry(0, 0, width, MainWindow::self()->height());
    setGeometry(openGeometry);

    QRect closeButtonGeometry(width, 0, MainWindow::self()->width() - width, MainWindow::self()->height());
    closeButton->setGeometry(closeButtonGeometry);
}

bool SidePanel::event(QEvent *event)
{
    return QWidget::event(event);
}

int SidePanel::calculateWidth() const {
    return qMin(MainWindow::self()->width() * .75, 300.0);
}

void SidePanel::touchEvent(QTouchEvent *event)
{
    const QList<QTouchEvent::TouchPoint> &touchPoints = event->points();
    const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();

    if (event->type() == QEvent::TouchBegin) {
        qDebug() << "touch begin";
        touchStartPoint = touchPoint.position().toPoint();
        swipeFromLeftDetected = false; // Reset detection flag
    } else if (event->type() == QEvent::TouchEnd) {
        int dx = touchPoint.position().x() - touchStartPoint.x();
        qDebug() << "touch end";

        // Consider it a "swipe from left" if the swipe started near the left edge
        // and moved rightward significantly
        if (touchStartPoint.x() < 50 && dx > 100) { // Threshold values, adjust as needed
            swipeFromLeftDetected = true;
            //            toggle(); // Your toggle function to show/hide the panel
            expand();
            qDebug() << "expanding side panel";
        }
    }
}


