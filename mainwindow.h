#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void consoleIn ( QString command );

private slots:
  void on_action_Exit_triggered();
  void toggle_widget( bool checked );

  void on_ConsoleIn_returnPressed();
  void conOut ( QString text );
  void conIn ( QString command );

private:
  Ui::MainWindow *ui;
  std::map<std::string, std::string> conVars;

  std::vector<std::string> parse_cmd ( std::string& command );
};

extern MainWindow* mainWindow;
#endif // MAINWINDOW_H
