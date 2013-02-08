#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include "widget.h"
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load("semiconductor_" + QLocale::system().name());
    a.installTranslator(&translator);

    Model model;

    Widget w(&model);
    w.show();
    return a.exec();
}
