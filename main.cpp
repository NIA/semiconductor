#include <QtGui/QApplication>
#include "widget.h"
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Model model;

    Widget w(&model);
    w.show();
    return a.exec();
}
