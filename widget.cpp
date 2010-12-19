#include "widget.h"
#include "ui_widget.h"

#include <qwt_plot.h>
#include <QDoubleSpinBox>
#include <QSpinBox>

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

    copySiliconFromModel();
    copyAdmixturesDefaultFromModel();
    copyOthersDefaultFromModel();
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

void Widget::copySiliconFromModel()
{
    findChild<QDoubleSpinBox*>("EgSpinner")->setValue(model->get_Eg_eV());
    findChild<QDoubleSpinBox*>("mcSpinner")->setValue(model->get_mc_m0());
    findChild<QDoubleSpinBox*>("mvSpinner")->setValue(model->get_mv_m0());
    findChild<QDoubleSpinBox*>("permittivitySpinner")->setValue(model->get_permittivity());
}

void Widget::copyAdmixturesDefaultFromModel()
{
    findChild<QDoubleSpinBox*>("EaSpinner")->setValue(model->get_Ea_eV());
    findChild<QDoubleSpinBox*>("EdSpinner")->setValue(model->get_Ed_eV());
    findChild<QDoubleSpinBox*>("NaMantissaSpinner")->setValue(double_mantissa(model->get_density_acceptor()));
    findChild<QSpinBox*>("NaExponentSpinner")->setValue(double_exponent(model->get_density_acceptor()));
    findChild<QDoubleSpinBox*>("NdMantissaSpinner")->setValue(double_mantissa(model->get_density_donor()));
    findChild<QSpinBox*>("NdExponentSpinner")->setValue(double_exponent(model->get_density_donor()));
}

void Widget::copyOthersDefaultFromModel()
{
    findChild<QDoubleSpinBox*>("TSpinner")->setValue(model->get_T());
    findChild<QDoubleSpinBox*>("surfacePotentialSpinner")->setValue(model->get_surface_potential());
}
