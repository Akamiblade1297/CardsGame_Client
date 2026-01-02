#ifndef REJOINDIALOG_H
#define REJOINDIALOG_H

#include <QDialog>
#include <QString>
#include "./ui_joindialog.h"

class JoinDialog : public QDialog {
    Q_OBJECT

public:
    explicit JoinDialog(QWidget* parent = nullptr);
    ~JoinDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    // void on_form_focusEvent();

private:
    Ui::JoinDialog* ui;
};

#endif
