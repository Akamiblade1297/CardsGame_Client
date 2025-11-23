#include "main.h"
#include "mainwindow.h"
#include "protocol.h"
#include "network.h"
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

  bool suc;
  Connection conn("127.0.0.1", 8494, &suc);
  if ( suc ) {
      protocol::ErrorCode res;
      char pass[8];
      pass[0] = 8;
      res = protocol::join(&conn, "A97", pass);
      for ( int i = 1 ; res == protocol::RENAME ; i++ ) {
        std::string name = "A97(" + std::to_string(i) + ")";
        res = protocol::join(nullptr, name.data(), pass);
      }
      std::cout << "Joined" << std::endl;
      if ( res == protocol::NOERROR ) res = protocol::cards("TREASURES", &table.Treasures.Cards);
      if ( res == protocol::NOERROR ) res = protocol::deck("TREASURES", "TABLE", "50", "60");
      if ( res == protocol::NOERROR ) res = protocol::chat("Ez, get better nigga");
      if ( res == protocol::NOERROR ) res = protocol::rotate("TABLE", "0", "42");
      if ( res == protocol::NOERROR ) res = protocol::action("making a SIGMA DEB");
      uint8_t rolls[3];
      rolls[0] = 3;
      if ( res == protocol::NOERROR ) res = protocol::roll(rolls, "6", "3");
    }
  return a.exec ();
}
