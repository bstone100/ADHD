#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include "QtWidgets/qboxlayout.h"
#include "QtWidgets/qpushbutton.h"
#include <QWidget>
#include "QTouchEvent"
#include "qelapsedtimer.h"
#include "qpropertyanimation.h"

class SidePanel : public QWidget {
    Q_OBJECT
public:
    explicit SidePanel(QWidget *parent = nullptr);

    static SidePanel *self();

    void expand();
    void collapse();
    void toggle();

    bool isExpanding();
    bool isCollapsing();
    bool isVisibleToUser();
    bool isExpanded(){return isPanelOpen;}

    void updateSize();
    QRect closedGeometry();
    QRect openGeometry();
    QRect closeButtonGeometry();

    QVBoxLayout *verticalLayout(){return vLayout;}

    void touchEvent(QTouchEvent *event);
protected:
    bool event(QEvent *event) override;

signals:
    void animationStarted(bool openStarted);
    void animationFinished(bool openFinished);

private:
    static SidePanel *singleton;

    QPushButton *closeButton;

    QVBoxLayout *vLayout;

    int calculateWidth() const;

    QPoint touchStartPoint;
    QPoint previousPoint;

    // v = x/t
    int dx;
    int dt;

    QElapsedTimer stopwatch;
    bool isPanelOpen;

    void handleSwipeEnd();

    QPropertyAnimation *expandAnimation;
    QPropertyAnimation *collapseAnimation;
};

#endif // SIDEPANEL_H


