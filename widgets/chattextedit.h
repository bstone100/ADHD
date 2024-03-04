#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit ChatTextEdit(QWidget *parent = nullptr);

    void updateHeight();

    int getMaxHeight() const;
    void setMaxHeight(int newMaxHeight);

    int getMinHeight() const;
    void setMinHeight(int newMinHeight);

private:
    int maxHeight;
    int minHeight;

};

#endif // CHATTEXTEDIT_H
