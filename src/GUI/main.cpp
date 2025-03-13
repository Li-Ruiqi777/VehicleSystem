#include "mainwindow.h"

#include <QApplication>

#include "plog/Appenders/ColorConsoleAppender.h"
#include "plog/Formatters/TxtFormatter.h"
#include "plog/Init.h"
#include "plog/Log.h"

int main(int argc, char *argv[])
{
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);
    PLOGI << "System initialized";

    QApplication a(argc, argv);
    MainWindow w;

    w.showMaximized();
    return a.exec();
}
