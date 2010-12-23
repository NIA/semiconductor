#include "widget.h"
#include "ui_widget.h"

#include <qwt_plot.h>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>

namespace
{
    int MIN_DENSITY_EXPONENT = 15;
    int MAX_DENSITY_EXPONENT = 20;

    double density_slider_value(QSlider * slider)
    {
        Q_ASSERT(slider->maximum() != slider->minimum());
        double fraction = float(slider->value() - slider->minimum())/(slider->maximum() - slider->minimum());
        double exponent = MIN_DENSITY_EXPONENT + fraction*(MAX_DENSITY_EXPONENT - MIN_DENSITY_EXPONENT);
        return pow(10, exponent);
    }

    void set_value_to_density_slider(QSlider * slider, double value)
    {
        Q_ASSERT(MAX_DENSITY_EXPONENT != MIN_DENSITY_EXPONENT);
        double fraction = (log10(value) - MIN_DENSITY_EXPONENT)/(MAX_DENSITY_EXPONENT - MIN_DENSITY_EXPONENT);
        double slider_value = slider->minimum() + fraction*(slider->maximum() - slider->minimum());
        slider->setValue(slider_value);
    }

    inline QString format_density(double value)
    {
        return QString::number(value, 'e', 1);
    }
}

Widget::Widget(Model * model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    model(model),
    initializing(true)
{
    ui->setupUi(this);
    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");

    curve = new QwtPlotCurve("Sine");
    curve->attach(plotArea);


    copySiliconFromModel();
    copyAdmixturesDefaultFromModel();
    copyOthersDefaultFromModel();

    initializing = false;

    refreshPlot();
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
    if(initializing)
        return;

    model->fill_data();
    model->get_bending_data_eV(plot_data);
    curve->setSamples(plot_data.xs, plot_data.ys);
    findChild<QwtPlot*>("plotArea")->replot();

    findChild<QLineEdit*>("fermiLevelLineEdit")->setText(QString::number(model->get_fermi_level_eV(), 'f', 6));
    findChild<QLineEdit*>("differenceLineEdit")->setText(QString::number(model->get_difference(), 'e', 2));
    findChild<QLineEdit*>("fieldLineEdit")->setText(QString::number(model->get_surface_field(), 'f', 2));
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
    set_value_to_density_slider(findChild<QSlider*>("NaSlider"), model->get_density_acceptor());
    set_value_to_density_slider(findChild<QSlider*>("NdSlider"), model->get_density_donor());
}

void Widget::copyOthersDefaultFromModel()
{
    findChild<QDoubleSpinBox*>("TSpinner")->setValue(model->get_T());
    findChild<QDoubleSpinBox*>("surfacePotentialSpinner")->setValue(model->get_surface_potential_volt());
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
    model->set_surface_potential_volt(value);
    refreshPlot();
}

void Widget::on_NaSlider_valueChanged(int)
{
    double Na_value = density_slider_value(reinterpret_cast<QSlider*>(sender()));
    findChild<QLineEdit*>("NaLineEdit")->setText(format_density(Na_value));

    model->set_density_acceptor(Na_value);
    refreshPlot();
}

void Widget::on_NdSlider_valueChanged(int)
{
    double Nd_value = density_slider_value(reinterpret_cast<QSlider*>(sender()));
    findChild<QLineEdit*>("NdLineEdit")->setText(format_density(Nd_value));

    model->set_density_donor(Nd_value);
    refreshPlot();
}
