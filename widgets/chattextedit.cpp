#include "chattextedit.h"
#include <QFontMetrics>
#include "QAbstractTextDocumentLayout"
#include "QScroller"

ChatTextEdit::ChatTextEdit(QWidget *parent) : QTextEdit(parent) {
    connect(this, &ChatTextEdit::textChanged, this, &ChatTextEdit::updateHeight);

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    QScroller* scroller = QScroller::scroller(this);

    QScrollerProperties properties = scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.0);

    scroller->setScrollerProperties(properties);
    scroller->grabGesture(this, QScroller::TouchGesture);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif

    minHeight = 40;
    maxHeight = 100;

    updateHeight();
}

void ChatTextEdit::updateHeight() {
    int docHeight = this->document()->size().height(); // Get the document height
    int margins = this->contentsMargins().top() + this->contentsMargins().bottom(); // Calculate the total vertical margins

    int height = qBound(minHeight, docHeight + margins, maxHeight);

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
    setVerticalScrollBarPolicy(height < maxHeight ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
#endif

    setFixedHeight(height);
}

int ChatTextEdit::getMaxHeight() const
{
    return maxHeight;
}

void ChatTextEdit::setMaxHeight(int newMaxHeight)
{
    maxHeight = newMaxHeight;
    updateHeight();
}

int ChatTextEdit::getMinHeight() const
{
    return minHeight;
}

void ChatTextEdit::setMinHeight(int newMinHeight)
{
    minHeight = newMinHeight;
    updateHeight();
}

bool ChatTextEdit::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
        updateHeight();
        break;
    default:
        break;
    }

    return QTextEdit::event(e);
}






