#include "so_gui_control.h"

#include <QApplication>

#ifdef _WIN32
  #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SO_GUI w;
    w.show();
    return a.exec();
}
