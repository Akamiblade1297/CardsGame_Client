#ifndef FORM_LINEEDIT_H
#define FORM_LINEEDIT_H

#include <QLineEdit>

class FormLineEdit : public QLineEdit {
    Q_OBJECT
public:
    FormLineEdit ( const QString& message, QWidget* parent )
        : QLineEdit(message, parent) {}
    FormLineEdit ( QWidget* parent )
        : QLineEdit(parent) {}

protected:
    void focusInEvent(QFocusEvent* event) override;
};

#endif
