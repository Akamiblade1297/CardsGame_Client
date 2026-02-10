#include "rejoindialog.h"
#include "../../ui/ui_rejoindialog.h"
#include "mainwindow.h"
#include <QDialogButtonBox>
#include <QPushButton>

RejoinDialog::RejoinDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::RejoinDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Перезайти");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
}
RejoinDialog::~RejoinDialog() {
    delete ui;
}

void RejoinDialog::on_buttonBox_accepted() {
    QString ip = ui->IPLineEdit->text();
    QString port = ui->PortLineEdit->text();

    emit mainWindow->consoleIn("connect "+ip+' '+port+" && rejoin $"+ip+':'+port, false);
    accept();
}

void RejoinDialog::on_buttonBox_rejected() {
    reject();
}
