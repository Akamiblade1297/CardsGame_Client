#include "formlineedit.h"
#include <QTimer>

void FormLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
}
