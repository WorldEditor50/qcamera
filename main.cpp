#include "mainwindow.h"
#include <QApplication>
#include "logger.hpp"
#include "singleapp.h"
#include "configuration.h"

int main(int argc, char *argv[])
{
    SingleApp app(argc, argv);

    if (app.isRunning() == true) {
        return 0;
    };
#ifdef Q_OS_ANDROID
    Configuration::instance().load();
#endif
    MainWindow w;
    app.setWindow(&w);
    w.showMaximized();
    return app.exec();
}
