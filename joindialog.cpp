#include "joindialog.h"
#include "./ui_joindialog.h"
#include "mainwindow.h"
#include <QDialogButtonBox>
#include <QPushButton>

JoinDialog::JoinDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::JoinDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Присоединиться");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
}
JoinDialog::~JoinDialog() {
    delete ui;
}

void JoinDialog::on_buttonBox_accepted() {
    QString ip = ui->IPLineEdit->text();
    QString port = ui->PortLineEdit->text();
    QString name = ui->NameLineEdit->text();

    emit mainWindow->consoleIn("connect "+ip+' '+port+" && join "+name+" > "+ip+':'+port, false);
    accept();
}

void JoinDialog::on_buttonBox_rejected() {
    reject();
}
