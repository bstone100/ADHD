#include "ChatTextEdit.h"
#include <QFontMetrics>
#include "QAbstractTextDocumentLayout"

ChatTextEdit::ChatTextEdit(QWidget *parent) : QTextEdit(parent) {
    connect(this, &ChatTextEdit::textChanged, this, &ChatTextEdit::updateHeight);

    minHeight = 40;
    maxHeight = 100;

    updateHeight();
}

void ChatTextEdit::updateHeight() {
    int docHeight = this->document()->size().height(); // Get the document height
    int margins = this->contentsMargins().top() + this->contentsMargins().bottom(); // Calculate the total vertical margins

    int height = qBound(minHeight, docHeight + margins, maxHeight);

    setFixedHeight(height);
}

int ChatTextEdit::getMaxHeight() const
{
    return maxHeight;
}

void ChatTextEdit::setMaxHeight(int newMaxHeight)
{
    maxHeight = newMaxHeight;
}

int ChatTextEdit::getMinHeight() const
{
    return minHeight;
}

void ChatTextEdit::setMinHeight(int newMinHeight)
{
    minHeight = newMinHeight;
}
