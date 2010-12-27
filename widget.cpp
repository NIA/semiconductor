#include "widget.h"
#include "ui_widget.h"

#include <qwt_plot.h>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>

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

    void configure_curve(QwtPlotCurve ** curve, QwtPlot * plot, const char* name, QColor color, Qt::PenStyle style = Qt::SolidLine)
    {
        (*curve) = new QwtPlotCurve(name);
        (*curve)->attach(plot);
        (*curve)->setPen(QPen(color, 1, style));
    }

    void set_level(QwtPlotCurve * curve, double value, double xmin, double xmax)
    {
        double xs[2] = {xmin, xmax};
        double ys[2] = {value, value};
        curve->setSamples(xs, ys, 2);
    }
}

Widget::Widget(Model * model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    model(model),
    initializing(true),
    acceptorEnabled(false),
    donorEnabled(true)
{
    ui->setupUi(this);
    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->setCanvasBackground(QColor(255,255,255));

    configure_curve(&bendingCurve, plotArea, "Bending", QColor(50, 50, 200));
    bendingCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    configure_curve(&EvCurve, plotArea, "Ev", Qt::black);
    configure_curve(&EcCurve, plotArea, "Ec", Qt::black);
    configure_curve(&EdCurve, plotArea, "Ed", Qt::black, Qt::DashLine);
    configure_curve(&EaCurve, plotArea, "Ea", Qt::black, Qt::DashLine);
    configure_curve(&fermiLevelCurve, plotArea, "Fermi Level", Qt::red, Qt::DashDotLine);

    copySiliconFromModel();
    copyAdmixturesDefaultFromModel();
    copyOthersDefaultFromModel();

    findChild<QCheckBox*>("acceptorCheckBox")->setChecked(acceptorEnabled);
    findChild<QCheckBox*>("donorCheckBox")->setChecked(donorEnabled);

    initializing = false;

    refreshPlot();
}

Widget::~Widget()
{
    delete bendingCurve;
    delete EvCurve;
    delete EcCurve;
    delete EdCurve;
    delete EaCurve;
    delete fermiLevelCurve;
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
    model->get_fermi_data_eV(plot_data);
    bendingCurve->setSamples(plot_data.xs, plot_data.ys);

    double xmin = 0;
    double xmax = 1;
    double Ev = 0;
    double Ec = Ev + model->get_Eg_eV();
    set_level(EvCurve, Ev, xmin, xmax);
    set_level(EcCurve, Ec, xmin, xmax);
    set_level(EaCurve, Ev + model->get_Ea_eV(), xmin, xmax);
    set_level(EdCurve, Ec - model->get_Ed_eV(), xmin, xmax);
    set_level(fermiLevelCurve, Ev + model->get_fermi_level_eV(), xmin, xmax);

    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->setAxisScale(QwtPlot::xBottom, 0, xmax);
    plotArea->replot();

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

void Widget::update_Na()
{
    double Na_value = density_slider_value(findChild<QSlider*>("NaSlider"));
    findChild<QLineEdit*>("NaLineEdit")->setText(format_density(Na_value));

    model->set_density_acceptor(Na_value);
}

void Widget::update_Nd()
{
    double Nd_value = density_slider_value(findChild<QSlider*>("NdSlider"));
    findChild<QLineEdit*>("NdLineEdit")->setText(format_density(Nd_value));

    model->set_density_donor(Nd_value);
}

void Widget::on_NaSlider_valueChanged(int)
{
    update_Na();
    refreshPlot();
}

void Widget::on_NdSlider_valueChanged(int)
{
    update_Nd();
    refreshPlot();
}

void Widget::on_acceptorCheckBox_stateChanged(int check_state)
{
    acceptorEnabled = (check_state == Qt::Checked);
    if(acceptorEnabled)
    {
        update_Na();
        EaCurve->attach(findChild<QwtPlot*>("plotArea"));
    }
    else
    {
        model->set_density_acceptor(0);
        EaCurve->detach();
    }

    findChild<QDoubleSpinBox*>("EaSpinner")->setEnabled(acceptorEnabled);
    findChild<QSlider*>("NaSlider")->setEnabled(acceptorEnabled);
    findChild<QLineEdit*>("NaLineEdit")->setEnabled(acceptorEnabled);
    refreshPlot();
}

void Widget::on_donorCheckBox_stateChanged(int check_state)
{
    donorEnabled = (check_state == Qt::Checked);
    if(donorEnabled)
    {
        update_Nd();
        EdCurve->attach(findChild<QwtPlot*>("plotArea"));
    }
    else
    {
        model->set_density_donor(0);
        EdCurve->detach();
    }

    findChild<QDoubleSpinBox*>("EdSpinner")->setEnabled(donorEnabled);
    findChild<QSlider*>("NdSlider")->setEnabled(donorEnabled);
    findChild<QLineEdit*>("NdLineEdit")->setEnabled(donorEnabled);
    refreshPlot();
}
