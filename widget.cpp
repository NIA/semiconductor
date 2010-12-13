#include "widget.h"
#include "ui_widget.h"
#include <qwt_plot.h>

Widget::Widget(Model * model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    model(model)
{
    ui->setupUi(this);
    QwtPlot * plotArea = findChild<QwtPlot*>("plotArea");
    curve = new QwtPlotCurve("Sine");
    const DataSeries & data = model->get_data();
    curve->setSamples(data.xs, data.ys);
    curve->attach(plotArea);
}

Widget::~Widget()
{
    delete curve;
    delete ui;
}

void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
