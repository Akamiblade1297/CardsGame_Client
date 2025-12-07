#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chatcompleter.h"
#include <string>
#include <QScrollBar>
#include <QShortcut>
#include <QScreen>
#include <QCompleter>
#include <QStringListModel>

#define TABL_ENABLED false
#define CHAT_ENABLED true
#define CONS_ENABLED true
#define SERV_ENABLED false

MainWindow::MainWindow (QWidget *parent)
    : QMainWindow (parent), ui (new Ui::MainWindow)
{
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

  // Splitter stretch factors
  ui->splitter_TaSL->setStretchFactor(0,3);
  ui->splitter_TaSL->setStretchFactor(1,1);

  ui->splitter_TaCC->setStretchFactor(0,5);
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
}

MainWindow::~MainWindow () { delete ui; }

void MainWindow::on_action_Exit_triggered()
{
  this->close();
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
