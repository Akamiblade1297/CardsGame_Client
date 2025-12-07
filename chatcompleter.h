#ifndef CHATCOMPLETER_H
#define CHATCOMPLETER_H

#include <QCompleter>

class ChatCompleter : public QCompleter {
    Q_OBJECT
public:
    using QCompleter::QCompleter;
protected:
    QStringList splitPath(const QString &path) const override;
    QString pathFromIndex(const QModelIndex &index) const override;
};

#endif // CHATCOMPLETER_H
