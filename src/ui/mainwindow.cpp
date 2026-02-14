#include "mainwindow.h"
#include "../../ui/ui_mainwindow.h"
#include "../other/network.h"
#include "../protocol/protocol.h"
#include "../protocol/deck.h"
#include "chatcompleter.h"
#include "rejoindialog.h"
#include "joindialog.h"
#include <iostream>
#include <QScrollBar>
#include <QShortcut>
#include <QScreen>
#include <QCompleter>
#include <QStringListModel>
#include <QFormLayout>
#include <QMessageBox>

#define TABL_ENABLED true
#define CHAT_ENABLED true
#define CONS_ENABLED true
#define SERV_ENABLED true

MainWindow::MainWindow (QWidget *parent)
    : QMainWindow (parent), ui (new Ui::MainWindow)
{
  Connection::NetworkInit();

  ui->setupUi (this);
  resize(screen()->availableGeometry().size() * 0.8);

  // Console scroll down automaticly
  QScrollBar* bar = ui->ConsoleScrollArea->verticalScrollBar();
  connect(bar, &QScrollBar::rangeChanged, this, &MainWindow::on_ConsoleVerticalScrollbar_rangeChanged);
  connect(bar, &QScrollBar::valueChanged, this, &MainWindow::on_ConsoleVerticalScrollbar_valueChanged);

  // Connect custom console signals
  connect(this, &MainWindow::consoleOut, this, &MainWindow::conOut);
  connect(this, &MainWindow::consoleIn, this, &MainWindow::conIn);

  // Setup console vars
  conPatternSet = false;
  scrollLocked  = false;
  std::ifstream conHistory(CONHISTORY, std::ios::ate);
  historyGetPointer = (int)conHistory.tellg()-1;
  historyGetPointerMax = historyGetPointer;
  conHistory.close();

  // Console custom console signals
  connect(this, &MainWindow::chatOut, this, &MainWindow::on_chatOut);

  // Shortcuts
  QShortcut* shortConsoleClear  = new QShortcut(QKeySequence("Ctrl+L"), ui->ConsoleIn);
  // QShortcut* shortConsoleCancel = new QShortcut(QKeySequence("Ctrl+U"), ui->ConsoleIn);
  // QShortcut* shortConsoleStart  = new QShortcut(QKeySequence("Ctrl+A"), ui->ConsoleIn);
  // QShortcut* shortConsoleEnd    = new QShortcut(QKeySequence("Ctrl+E"), ui->ConsoleIn);
  QShortcut* shortConsoleHistoryUp   = new QShortcut(QKeySequence(Qt::Key_Up  ), ui->ConsoleIn);
  QShortcut* shortConsoleHistoryDown = new QShortcut(QKeySequence(Qt::Key_Down), ui->ConsoleIn);

  shortConsoleClear ->setContext(Qt::WidgetShortcut);
  // shortConsoleCancel->setContext(Qt::WidgetShortcut);
  // shortConsoleStart ->setContext(Qt::WidgetShortcut);
  // shortConsoleEnd   ->setContext(Qt::WidgetShortcut);
  shortConsoleHistoryUp  ->setContext(Qt::WidgetShortcut);
  shortConsoleHistoryDown->setContext(Qt::WidgetShortcut);

  connect(shortConsoleClear, &QShortcut::activated, this, [this]{
      this->ui->ConsoleOut->clear();
  });
  // connect(shortConsoleCancel, &QShortcut::activated, this, [this]{
      // this->ui->ConsoleIn->clear();
  // });
  // connect(shortConsoleStart, &QShortcut::activated, this, [this]{
    // this->ui->ConsoleIn->setCursorPosition(0);
  // });
  // connect(shortConsoleEnd, &QShortcut::activated, this, [this]{
    // this->ui->ConsoleIn->setCursorPosition(this->ui->ConsoleIn->text().length());
  // });
  connect(shortConsoleHistoryUp, &QShortcut::activated, this, [this]{
      this->conHistoryFind(true);
  });
  connect(shortConsoleHistoryDown, &QShortcut::activated, this, [this]{
      this->conHistoryFind(false);
  });

  // Chat variables
  QStringList chat_cmdList = CHAT_CMDS;
  chatCompleterModel = new QStringListModel(chat_cmdList);
  ChatCompleter* chatCompleter = new ChatCompleter(chatCompleterModel, ui->ChatIn);
  chatCompleter->setCompletionMode(QCompleter::PopupCompletion);
  ui->ChatIn->setCompleter(chatCompleter);

  // ServerList Context Menu
  connect(ui->ServerList, &QWidget::customContextMenuRequested, this, &MainWindow::ServerListContextMenu);
  subnet_addr = 0xC0A80000; // 192.168.0.0
  subnet_mask = 0xFFFFFF00;
  UpdateServerList();

  // Splitter stretch factors
  ui->splitter_TaSL->setStretchFactor(0,3);
  ui->splitter_TaSL->setStretchFactor(1,1);

  ui->splitter_TaCC->setStretchFactor(0,30);
  ui->splitter_TaCC->setStretchFactor(1,2);

  ui->splitter_CC->setStretchFactor(0,1);
  ui->splitter_CC->setStretchFactor(1,1);

  // Menubar actions
  connect(ui->action_Chat,        SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_Table,       SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_Console,     SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_ServerList,  SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));

  ui->action_Table     ->setChecked(TABL_ENABLED);ui->Table     ->setVisible(TABL_ENABLED);
  ui->action_Chat      ->setChecked(CHAT_ENABLED);ui->Chat      ->setVisible(CHAT_ENABLED);
  ui->action_Console   ->setChecked(CONS_ENABLED);ui->Console   ->setVisible(CONS_ENABLED);
  ui->action_ServerList->setChecked(SERV_ENABLED);ui->ServerList->setVisible(SERV_ENABLED);

  Table     = CardContainer(ui->TableFrame);
  Treasures = Deck(ui->TableFrame, ui->Treasures);
  Trapdoors = Deck(ui->TableFrame, ui->Trapdoors);

  // for ( int i = 0 ; i < 90 ; i++ ) {
  //     CardFrame* frame = new CardFrame(ui->TableFrame);
  //     Card* card = new Card(TRAP, frame);
  //     Trapdoors.push(card);
  // }
}

MainWindow::~MainWindow () {
    Connection::NetworkClean();
    delete ui;
}

void MainWindow::on_action_Exit_triggered()
{
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("УВЕРЕН????");
  msgBox.setText("Вы уверены, что хотите полностью закрыть игру?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  msgBox.button(QMessageBox::Yes)->setText("Да");
  msgBox.button(QMessageBox::No)->setText("Нет");
  if ( msgBox.exec() == QMessageBox::Yes )
      this->close();
}
void MainWindow::on_action_Join_triggered() {
    JoinDialog* dialog = new JoinDialog(this);
    dialog->exec();
}
void MainWindow::on_action_Rejoin_triggered() {
    RejoinDialog* dialog = new RejoinDialog(this);
    dialog->exec();
}
void MainWindow::on_action_Disconnect_triggered() {
    if ( protocol::connection() != nullptr ) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("УВЕРЕН????");
        msgBox.setText("Вы уверены, что хотите отключиться от сервера?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.button(QMessageBox::Yes)->setText("Да");
        msgBox.button(QMessageBox::No)->setText("Нет");

       if ( msgBox.exec() == QMessageBox::Yes ) emit consoleIn("disconnect", false);
    }
}

void MainWindow::toggle_widget ( bool checked ) {
  QObject* sender = this->sender();
  if ( sender == ui->action_Chat ) {
      ui->Chat->setVisible(checked);
  } else if ( sender == ui->action_Table ) {
      ui->Table->setVisible(checked);
  } else if ( sender == ui->action_Console ) {
      ui->Console->setVisible(checked);
  } else if ( sender == ui->action_ServerList )  {
      ui->ServerList->setVisible(checked);
  }
}

CardContainer* MainWindow::checkContainer( QPoint& point ) const {
    if ( ui->Treasures->rect().contains(ui->Treasures->mapFromGlobal(point)) ) {
        return &Treasures;
    } else if ( ui->Trapdoors->rect().contains(ui->Trapdoors->mapFromGlobal(point)) ) {
        return &Trapdoors;
    } else {
        return &Table;
    }
}
