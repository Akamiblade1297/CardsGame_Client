#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow (QWidget *parent)
    : QMainWindow (parent), ui (new Ui::MainWindow)
{
  ui->setupUi (this);

  ui->action_Table     ->setChecked(true);
  ui->action_Chat      ->setChecked(true);
  ui->action_Console   ->setChecked(true);
  ui->action_ServerList->setChecked(true);

  ui->splitter_TaSL->setStretchFactor(0,3);
  ui->splitter_TaSL->setStretchFactor(1,1);

  ui->splitter_TaCC->setStretchFactor(0,5);
  ui->splitter_TaCC->setStretchFactor(1,2);

  ui->splitter_CC->setStretchFactor(0,1);
  ui->splitter_CC->setStretchFactor(1,1);

  ui->statusbar->showMessage("Some status", -1);
}

MainWindow::~MainWindow () { delete ui; }

void MainWindow::on_action_Exit_triggered()
{
  this->close();
}

