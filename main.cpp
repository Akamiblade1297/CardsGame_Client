#include "main.h"
#include "mainwindow.h"
#include "protocol.h"
#include "network.h"
#include <chrono>
#include <thread>
#include <iostream>

#include <QApplication>
Table table;
PlayerManager playerMgr;

int
main (int argc, char *argv[])
{
  Connection::NetworkInit();
  QApplication a (argc, argv);
  MainWindow w;
  w.show ();

  return a.exec ();
}
