#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("办公助手");
    a.setOrganizationName("OfficeAssistant");
    a.setApplicationVersion("1.0");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
