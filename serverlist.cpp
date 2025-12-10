#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocol.h"
#include <functional>
#include <iostream>
#include <QInputDialog>

void MainWindow::on_joinServerButton_pressed() {
    QObject* sender = QObject::sender();
    bool ok;
    QString username = QInputDialog::getText(this, "Username", "Enter your Username:", QLineEdit::Normal, "", &ok);
    if ( ok && !username.isEmpty() )
        consoleIn("connect "+sender->property("ip").toString()+" "+sender->property("port").toString()+" && join "+username, false);
}

void MainWindow::addServer ( const char* ip, unsigned short port ) {
    if ( protocol::connect(ip, port) != protocol::_NOERROR ) return;

    QString text = QString(ip);
    if ( port != 8494 ) text += ':' + QString::number(port);
    QLabel* serverAddressLabel = new QLabel(text, ui->ServerList);
    serverAddressLabel->setMinimumWidth(150);
    QPushButton* serverJoinButton = new QPushButton("Join", ui->ServerList);
    serverJoinButton->setMinimumWidth(50);
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
            ServerScanner::scanSubnet(0xC0A80000, 0xFFFFFF00); // Scan subnet 192.168.0.0/24
    for ( int i = 0 ; i < servers.size() ; i++ ) {
        char ip[INET_ADDRSTRLEN];
#ifdef _WIN32
        inet_ntop(AF_INET, &servers[i], buffer, INET_ADDRSTRLEN);
#else
        struct in_addr addr;
        addr.s_addr = htonl(servers[i]);
        inet_ntop(AF_INET, &addr, ip, INET_ADDRSTRLEN);
#endif
        addServer(ip, 8494);
    }
}

void MainWindow::ServerListContextMenu( const QPoint &pos ) {
    QMenu menu;
    menu.addAction("Refresh", this, &MainWindow::UpdateServerList);
    menu.exec(ui->ServerList->mapToGlobal(pos));
}
