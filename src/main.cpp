#include "ui/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <string>
#include <filesystem>

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string gameDir;
MainWindow* mainWindow;

void getGameDirectory () {
#if _WIN32
    wchar_t path[MAX_PATH];
    DWORD count = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (count != 0) {
        // Convert to UTF-8
        int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        std::string utf8Path(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &utf8Path[0], size, NULL, NULL);
        utf8Path.pop_back(); // Remove null terminator
        gameDir = utf8Path;
    }
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        gameDir = std::string(path, count);
    }
#endif
    if ( !gameDir.empty() ) {
        gameDir = std::filesystem::path(gameDir).parent_path().string();
#ifdef _WIN32
        gameDir += '\\';
#else
        gameDir += '/';
#endif
    }
}

int main (int argc, char *argv[])
{
  qRegisterMetaType<QPixmap>();
  getGameDirectory();

  QApplication a (argc, argv);
  MainWindow w;
  mainWindow = &w;
  w.show();

  return a.exec ();
}
