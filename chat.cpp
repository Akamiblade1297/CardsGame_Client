#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "string_func.h"
#include "main.h"
#include <QCompleter>
#include <QAbstractItemView>

namespace {
QString parse_msg ( std::string msg ) {
    for ( int i = 0 ; i < msg.size() ; i++ ) {
        if ( msg[i] == '\'' ) {
            msg.insert(i, 1, '\\');
            i++;
        }
    }
    return QString::fromStdString("'" + msg + "'");
}
QString parse_msg ( QString msg ) {
    for ( int i = 0 ; i < msg.size() ; i++ ) {
        if ( msg[i] == '\'' ) {
            msg.insert(i, '\\');
            i++;
        }
    }
    return "'" + msg + "'";
}
}

void MainWindow::on_chatOut( QString text ) {
    ui->ChatOut->setText(ui->ChatOut->text()+"<br>"+text);
}

void MainWindow::on_ChatIn_returnPressed() {
    QString message = ui->ChatIn->text();
    if ( message.isEmpty() ) return;

    ui->ChatIn->setText("");
    if ( message[0] == '/' ) {
        std::vector<std::string> msg = split(message.toStdString().data(), ' ');
        if ( msg[0] == "/me" ) consoleIn("act "+parse_msg(message.mid(4)), false);
        else if ( msg[0] == "/r" || msg[0] == "/roll" ) consoleIn(QString::fromStdString("roll "+msg[1]+' '+msg[2]), false);
        else if ( msg[0] == "/w" || msg[0] == "/whisper" ) {
            int mpos = msg[0].size() + msg[1].size() + 2;
            consoleIn("whisper "+parse_msg(msg[1])+" "+parse_msg(message.mid(mpos)), false);
        }
        else chatOut(QString::fromStdString("Unknown command \""+msg[0].substr(1,msg[0].size()-1)+'"'));
    } else consoleIn("chat "+parse_msg(message), false);
}

void MainWindow::on_ChatIn_textEdited(const QString &text) {
    QCompleter* completer = ui->ChatIn->completer();
    if ( text.contains(' ') ) {
        int i = text.indexOf(' ');
        QString ltext = text.left(i);
        QString rtext = text.mid(i+1);
        i = text.lastIndexOf(' ')+1;

        if ( ( ltext != "/whisper" && ltext != "/w" ) || i > text.size() || rtext.contains(' ') ) {
            chatCompleterModel->setStringList({});
            completer->setCompletionPrefix("");
        } else {
            QStringList playerlist;
            for ( int i = 1 ; i < playerMgr.Players.size() ; i++ ) {
                playerlist << QString::fromStdString(playerMgr.Players[i].Name);
            } chatCompleterModel->setStringList(playerlist);
            completer->setCompletionPrefix(rtext);
            if ( completer->completionCount() > 0 )
                completer->complete();
        }
    } else {
        QStringList cmdlist = CHAT_CMDS;
        chatCompleterModel->setStringList(cmdlist);
        completer->setCompletionPrefix(text);
        if ( text.length() > 0 && completer->completionCount() > 0 )
            completer->complete();
    }
}
