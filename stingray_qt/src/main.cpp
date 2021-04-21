#include "sr_control.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SR w;
    w.show();
    return a.exec();
}
