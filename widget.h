#pragma once

#include <QWidget>
#include <qwt_plot_curve.h>
#include "model.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget {
    Q_OBJECT
public:
    Widget(Model * model, QWidget *parent = 0);
    ~Widget();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Widget *ui;
    Model * model;
    QwtPlotCurve * curve;

    void copySiliconFromModel();
    void copyAdmixturesDefaultFromModel();
    void copyOthersDefaultFromModel();

};
