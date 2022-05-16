#include "mainwindow.h"
#include <QApplication>
#include "logger.hpp"
#include "singleapp.h"
#include "configuration.h"

int main(int argc, char *argv[])
{
    SingleApp a(argc, argv);

    if (a.isRunning() == true) {
        return 0;
    };
#ifdef Q_OS_ANDROID
    Configuration::instance().load();
#endif
    MainWindow w;
    a.setWindow(&w);
    w.showMaximized();
    return a.exec();
}
