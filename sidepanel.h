#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include "QtWidgets/qboxlayout.h"
#include "QtWidgets/qpushbutton.h"
#include <QWidget>

class SidePanel : public QWidget {
    Q_OBJECT
public:
    explicit SidePanel(QWidget *parent = nullptr);

    static SidePanel *self();

    void toggle();

    void updateSize();

    QVBoxLayout *verticalLayout(){return vLayout;}

signals:
    void animationStarted(bool openStarted);
    void animationFinished(bool openFinished);

private:
    static SidePanel *singleton;

    QPushButton *closeButton;

    QVBoxLayout *vLayout;

    int calculateWidth() const;
};

#endif // SIDEPANEL_H
