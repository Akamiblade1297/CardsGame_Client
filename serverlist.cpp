#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocol.h"
#include <functional>
#include <iostream>
#include <cstdio>
#include <QInputDialog>
#include <QMessageBox>

void MainWindow::on_joinServerButton_pressed() {
    QObject* sender = QObject::sender();
    QString ip = sender->property("ip").toString();
    QString port = sender->property("port").toString();
    bool success;
    char user[21]; user[0] = 1;
    emit consoleIn("print "+ip+":"+port+"@user", false, &success, user, 21);
    if ( success ) {
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("Dejavu");
        msgBox->setText("Похоже вы уже были на этом сервере\nХотите перезайти как "+QString::fromUtf8(user)+'?');
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        msgBox->button(QMessageBox::Yes)->setText("Да");
        msgBox->button(QMessageBox::No)->setText("Нет");
        if ( msgBox->exec() == QMessageBox::Yes ) {
            consoleIn("connect "+ip+" "+port+" && rejoin $"+ip+":"+port, false);
            return;
        }
    }
    QInputDialog* nameDialog = new QInputDialog(this);
    nameDialog->setWindowTitle("Никнейм");
    nameDialog->setLabelText("Введите свой никнейм");
    nameDialog->setTextEchoMode(QLineEdit::Normal);
    nameDialog->setOkButtonText("Ок");
    nameDialog->setCancelButtonText("Отмена");
    if ( nameDialog->exec() == QDialog::Accepted && !nameDialog->textValue().isEmpty() )
        consoleIn("connect "+ip+" "+port+" && join "+nameDialog->textValue()+" > "+ip+":"+port+" && "+ip+":"+port+"@user = "+nameDialog->textValue(), false);
}

void MainWindow::addServer ( const char* ip, unsigned short port ) {
    QString text = QString(ip);
    if ( port != 8494 ) text += ':' + QString::number(port);
    QLabel* serverAddressLabel = new QLabel(text, ui->ServerList);
    serverAddressLabel->setMinimumWidth(140);
    QPushButton* serverJoinButton = new QPushButton("Join", ui->ServerList);
    serverJoinButton->setMinimumWidth(60);
    serverJoinButton->setProperty("ip", ip);
    serverJoinButton->setProperty("port", port);

    connect(serverJoinButton, &QPushButton::pressed, this, &MainWindow::on_joinServerButton_pressed);

    QFormLayout* serverListLayout = qobject_cast<QFormLayout*>(ui->ServerList->layout());
    serverListLayout->addRow(serverAddressLabel, serverJoinButton);

}

void MainWindow::clearServers() {
    QFormLayout* serverListLayout = qobject_cast<QFormLayout*>(ui->ServerList->layout());
    while ( serverListLayout->rowCount() > 0 ) {
        serverListLayout->removeRow(serverListLayout->rowCount()-1);
    }
}

void MainWindow::UpdateServerList() {
    clearServers();
    std::vector<uint32_t> servers =
            ServerScanner::scanSubnet(subnet_addr, subnet_mask);
    for ( int i = 0 ; i < servers.size() ; i++ ) {
        char ip[INET_ADDRSTRLEN];
#ifdef _WIN32
        inet_ntop(AF_INET, &servers[i], ip, INET_ADDRSTRLEN);
#else
        struct in_addr addr;
        addr.s_addr = htonl(servers[i]);
        inet_ntop(AF_INET, &addr, ip, INET_ADDRSTRLEN);
#endif
        addServer(ip, 8494);
    }
}

void MainWindow::UpdateServerListIn() {
    bool ok;
    QInputDialog* subnetDialog = new QInputDialog(this);
    subnetDialog->setWindowTitle("Подсеть");
    subnetDialog->setLabelText("Введите подсеть");
    subnetDialog->setTextEchoMode(QLineEdit::Normal);
    subnetDialog->setOkButtonText("Ок");
    subnetDialog->setCancelButtonText("Отмена");

    if ( subnetDialog->exec() != QDialog::Accepted ) return;
    std::string text = subnetDialog->textValue().toStdString();

    if ( text == "127.0.0.1" || text == "localhost" ) {
      subnet_addr = 0x7F000001;
      subnet_mask = 0xFFFFFFFF;
    } else {
        unsigned int b1, b2, b3, b4, m;
        b1=0;b2=0;b3=0;b4=0;m=0;
        sscanf(text.data(), "%hhu.%hhu.%hhu.%hhu/%hhu", &b1, &b2, &b3, &b4, &m);
        subnet_addr = (b1<<24) + (b2<<16) + (b3<<8) + b4;
        subnet_mask = -1<<(32-m);
    }
    UpdateServerList();
}

void MainWindow::ServerListContextMenu( const QPoint &pos ) {
    QMenu menu;
    menu.addAction("Обновить", this, &MainWindow::UpdateServerList);
    menu.addAction("Обновить в..", this, &MainWindow::UpdateServerListIn);
    menu.exec(ui->ServerList->mapToGlobal(pos));
}
