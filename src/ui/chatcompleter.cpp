#include "chatcompleter.h"
#include <QLineEdit>
#include <stdexcept>

QStringList ChatCompleter::splitPath(const QString &path) const {
    QLineEdit* chatIn = qobject_cast<QLineEdit*>(widget());
    if ( chatIn == nullptr ) throw std::runtime_error("ChatCompleter's widget must be QLineEdit");

    int cursor = chatIn->cursorPosition();
    QString text = chatIn->text();

    int start = text.lastIndexOf(' ', cursor-1)+1;
    return {text.mid(start, cursor-start)};
}

QString ChatCompleter::pathFromIndex(const QModelIndex &index) const {
    QLineEdit* chatIn = qobject_cast<QLineEdit*>(widget());
    if ( chatIn == nullptr ) throw std::runtime_error("ChatCompleter's widget must be QLineEdit");

    QString completion = QCompleter::pathFromIndex(index);
    QString text = chatIn->text();
    int cursor = chatIn->cursorPosition();

    int start = text.lastIndexOf(' ', cursor-1) + 1;
    int end   = text.indexOf(' ', cursor);
    if ( end == -1 ) end = text.length();

    return text.left(start) + completion + text.mid(end);
}
