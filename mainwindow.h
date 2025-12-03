#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <fstream>

#define CONHISTORY gameDir+".console_history"

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

    /**
     * @brief Interpret a console command
     * @param command
     * @return boolean Success and string Out
     */
    std::pair<bool, std::string> conInterpret ( std::string command );

signals:
    void consoleOut ( QString text );
    void consoleIn ( QString command, bool user );

private slots:
  void on_action_Exit_triggered();
  void toggle_widget( bool checked );

  void conOut ( QString text );
  void conIn ( QString command, bool user );
  void conHistoryFind ( bool up );

  void on_ConsoleIn_returnPressed();
  void on_ConsoleIn_textEdited();
  void on_ConsoleVerticalScrollbar_rangeChanged();
  void on_ConsoleVerticalScrollbar_valueChanged();

private:
  Ui::MainWindow *ui;
  std::map<std::string, std::string> conVars;
  std::string conPattern;
  int historyGetPointer;
  int historyGetPointerMax;
  bool conPatternSet;
  bool scrollLocked;

  std::vector<std::string> parse_cmd ( std::string& command );
  bool conHistoryUp ( std::ifstream* conHistory );
  bool conHistoryDown ( std::ifstream* conHistory );
};

extern MainWindow* mainWindow;
extern std::string gameDir;
#endif // MAINWINDOW_H
