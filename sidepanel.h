#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include "QtWidgets/qboxlayout.h"
#include "QtWidgets/qpushbutton.h"
#include <QWidget>
#include "QTouchEvent"

class SidePanel : public QWidget {
    Q_OBJECT
public:
    explicit SidePanel(QWidget *parent = nullptr);

    static SidePanel *self();

    void expand();
    void collapse();
    void toggle();

    void updateSize();

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
    bool swipeFromLeftDetected = false;
};

#endif // SIDEPANEL_H
