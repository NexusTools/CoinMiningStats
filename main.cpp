#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/bitcoin.png"));
    a.setOrganizationName("NexusTools");
    a.setOrganizationDomain("net.nexustools");
    a.setApplicationName("BitMinerStats");

    MainWindow w;
    if(a.arguments().contains("-m"))
        w.showMinimized();
    else
        w.show();
    
    return a.exec();
}
