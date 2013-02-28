#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("NexusTools");
    a.setOrganizationDomain("net.nexustools");
    a.setApplicationName("BitMinerStats");

    MainWindow w;
    w.show();
    
    return a.exec();
}
