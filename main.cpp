#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedHeight(480);
    w.setFixedWidth(935);
    w.show();

    return a.exec();
}
