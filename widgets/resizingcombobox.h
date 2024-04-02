#ifndef RESIZINGCOMBOBOX_H
#define RESIZINGCOMBOBOX_H

#include <QComboBox>
#include <QWidget>

class ResizingComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ResizingComboBox(QWidget *parent = nullptr);

protected slots:
    void adjustWidth();

protected:
    void showPopup() override;
    void hidePopup() override;
private:
    int maximumItemWidth() const;
};

#endif // RESIZINGCOMBOBOX_H
