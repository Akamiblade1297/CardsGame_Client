#include "main.h"
#include "mainwindow.h"
#include "network.h"
#include <string>

#include <QApplication>
Table table;
PlayerManager playerMgr;
MainWindow* mainWindow;

int
main (int argc, char *argv[])
{
  Connection::NetworkInit();
  QApplication a (argc, argv);
  MainWindow w;
  w.show();

  mainWindow = &w;
  return a.exec ();
}
