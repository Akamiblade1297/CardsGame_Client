#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "network.h"

MainWindow::MainWindow (QWidget *parent)
    : QMainWindow (parent), ui (new Ui::MainWindow)
{
  ui->setupUi (this);

  ui->splitter_TaSL->setStretchFactor(0,3);
  ui->splitter_TaSL->setStretchFactor(1,1);

  ui->splitter_TaCC->setStretchFactor(0,5);
  ui->splitter_TaCC->setStretchFactor(1,2);

  ui->splitter_CC->setStretchFactor(0,1);
  ui->splitter_CC->setStretchFactor(1,1);

  connect(ui->action_Chat,        SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_Table,       SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_Console,     SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));
  connect(ui->action_ServerList,  SIGNAL(toggled(bool)), this, SLOT(toggle_widget(bool)));

  ui->action_Table     ->setChecked(true);ui->Table     ->setVisible(true);
  ui->action_Chat      ->setChecked(true);ui->Chat      ->setVisible(true);
  ui->action_Console   ->setChecked(true);ui->Console   ->setVisible(true);
  ui->action_ServerList->setChecked(true);ui->ServerList->setVisible(true);

  ui->statusbar->showMessage("Some status", -1);

  NetworkInit();
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

