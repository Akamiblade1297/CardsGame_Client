#ifndef JOINDIALOG_H
#define JOINDIALOG_H

#include <QDialog>
#include <QString>
#include "./ui_rejoindialog.h"

class RejoinDialog : public QDialog {
    Q_OBJECT

public:
    explicit RejoinDialog(QWidget* parent = nullptr);
    ~RejoinDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    // void on_form_focusEvent();

private:
    Ui::RejoinDialog* ui;
};

#endif
