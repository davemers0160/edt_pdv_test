#include "sr_control.h"

#include <QApplication>

#ifdef _WIN32
  #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SR w;
    w.show();
    return a.exec();
}
