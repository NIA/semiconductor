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
    curve = new QwtPlotCurve("Sine");
    curve->attach(findChild<QwtPlot*>("plotArea"));

    refreshPlot();

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
void Widget::refreshPlot()
{
    model->fill_data();
    const DataSeries & data = model->get_data();
    curve->setSamples(data.xs, data.ys);
    findChild<QwtPlot*>("plotArea")->replot();
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

void Widget::on_siliconButton_clicked()
{
    model->set_silicon();
    copySiliconFromModel();
}

void Widget::on_admixturesDefaultButton_clicked()
{
    model->set_admixtures_default();
    copyAdmixturesDefaultFromModel();
}

void Widget::on_othersDefaultButton_clicked()
{
    model->set_others_default();
    copyOthersDefaultFromModel();
}

void Widget::on_EgSpinner_valueChanged(double value)
{
    model->set_Eg_eV(value);
    refreshPlot();
}

void Widget::on_mcSpinner_valueChanged(double value)
{
    model->set_mc_m0(value);
    refreshPlot();
}

void Widget::on_mvSpinner_valueChanged(double value)
{
    model->set_mv_m0(value);
    refreshPlot();
}

void Widget::on_permittivitySpinner_valueChanged(double value)
{
    model->set_permittivity(value);
    refreshPlot();
}

void Widget::on_EaSpinner_valueChanged(double value)
{
    model->set_Ea_eV(value);
    refreshPlot();
}

void Widget::on_EdSpinner_valueChanged(double value)
{
    model->set_Ed_eV(value);
    refreshPlot();
}

void Widget::on_TSpinner_valueChanged(double value)
{
    model->set_T(value);
    refreshPlot();
}

void Widget::on_surfacePotentialSpinner_valueChanged(double value)
{
    model->set_surface_potential(value);
    refreshPlot();
}

void Widget::on_NaMantissaSpinner_valueChanged(double mantissa)
{
    double value = build_double(mantissa, findChild<QSpinBox*>("NaExponentSpinner")->value());
    model->set_density_acceptor(value);
    refreshPlot();
}

void Widget::on_NaExponentSpinner_valueChanged(int exponent)
{
    double value = build_double(findChild<QDoubleSpinBox*>("NaMantissaSpinner")->value(), exponent);
    model->set_density_acceptor(value);
    refreshPlot();
}

void Widget::on_NdMantissaSpinner_valueChanged(double mantissa)
{
    double value = build_double(mantissa, findChild<QSpinBox*>("NdExponentSpinner")->value());
    model->set_density_donor(value);
    refreshPlot();
}

void Widget::on_NdExponentSpinner_valueChanged(int exponent)
{
    double value = build_double(findChild<QDoubleSpinBox*>("NdMantissaSpinner")->value(), exponent);
    model->set_density_donor(value);
    refreshPlot();
}
