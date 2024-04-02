#include "resizingcombobox.h"
#include <QFontMetrics>
#include "QAbstractItemView"
#include "QScrollBar"

ResizingComboBox::ResizingComboBox(QWidget *parent) : QComboBox(parent) {
    connect(this, &ResizingComboBox::currentTextChanged, this, &ResizingComboBox::adjustWidth);
}

void ResizingComboBox::adjustWidth() {
    QFontMetrics metrics(font());
    int width = metrics.horizontalAdvance(currentText()) + 25;
    setFixedWidth(width);
}

int ResizingComboBox::maximumItemWidth() const {
    int maxWidth = 0;
    QFontMetrics metrics(font());
    for (int i = 0; i < count(); ++i) {
        maxWidth = qMax(maxWidth, metrics.horizontalAdvance(itemText(i)));
    }
    return maxWidth;
}

void ResizingComboBox::showPopup() {
    // Adjust the width of the popup before showing it
    QComboBox::showPopup();
    QAbstractItemView *popup = view();
    if (popup) {
        const int scrollbarWidth = popup->verticalScrollBar()->isVisible()
                                       ? popup->verticalScrollBar()->width() : 0;
        const int popupWidth = maximumItemWidth() + scrollbarWidth;
        popup->setFixedWidth(popupWidth * 1.5);
    }
    adjustWidth();
}

void ResizingComboBox::hidePopup() {
    QComboBox::hidePopup();
    adjustWidth();
}
