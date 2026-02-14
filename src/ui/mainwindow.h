#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QStringListModel>
#include <fstream>
#include "cardframe.h"

#define CONHISTORY gameDir+".console_history"
#define CHAT_CMDS { "/me", "/whisper", "/roll", "/clear" }

class CardContainer;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();
  CardContainer* checkContainer ( QPoint& point ) const;
  CardFrame* createCardFrame();

signals:
    void consoleOut ( QString text );
    void consoleIn ( QString command, bool user, bool* succ = nullptr, void* res = nullptr, size_t rsize = 0 );

    void chatOut ( QString text );

private slots:
  void on_action_Exit_triggered();
  void on_action_Join_triggered();
  void on_action_Rejoin_triggered();
  void on_action_Disconnect_triggered();
  void toggle_widget( bool checked );

  // Console
  void conOut ( QString text );
  void conIn ( QString command, bool user, bool* succ = nullptr, void* res = nullptr, size_t rsize = 0 );
  void conHistoryFind ( bool up );

  void on_ConsoleIn_returnPressed();
  void on_ConsoleIn_textEdited();
  void on_ConsoleVerticalScrollbar_rangeChanged();
  void on_ConsoleVerticalScrollbar_valueChanged();

  // Chat
  void on_chatOut ( QString text );

  void on_ChatIn_returnPressed();
  void on_ChatIn_textEdited(const QString &arg1);

  // ServerList
  void ServerListContextMenu ( const QPoint& pos );
  void on_joinServerButton_pressed();

private:
  Ui::MainWindow *ui;
  // Console
  std::map<std::string, std::string> conVars;
  std::string conPattern;
  int historyGetPointer;
  int historyGetPointerMax;
  bool conPatternSet;
  bool scrollLocked;

  std::vector<std::string> parse_cmd ( std::string& command );
  bool conHistoryUp ( std::ifstream* conHistory );
  bool conHistoryDown ( std::ifstream* conHistory ); 
  std::string conInterpret ( std::string command, bool* succ, void* result, size_t rsize );
  // Chat
  QStringListModel* chatCompleterModel;
  // ServerList
  int subnet_addr, subnet_mask;

  void addServer ( const char* ip, unsigned short port );
  void UpdateServerList();
  void UpdateServerListIn();
  void clearServers();
};

extern MainWindow* mainWindow;
extern std::string gameDir;
#endif // MAINWINDOW_H
