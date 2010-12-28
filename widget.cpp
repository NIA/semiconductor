#include "widget.h"
#include "ui_widget.h"

#include <qwt_plot.h>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>

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

    enum PlotVariant
    {
        PV_FERMI_DISTRIBUTION,
        PV_FERMI_LEVEL,
        PV_NA,
        PV_ND,
        PV_N,
        PV_P,
        _PV_COUNT
    };

    const QString plot_captions[_PV_COUNT] =
    {
        "Fermi distribution(Energy)",
        "Fermi level(1/kT)",
        "Charged acceptors(1/kT)",
        "Charged donors(1/kT)",
        "Free electrons(1/kT)",
        "Free holes(1/kT)"
    };
}

Widget::Widget(Model * model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    model(model),
    initializing(true),
    acceptorEnabled(false),
    donorEnabled(true),
    plotVariant(PV_FERMI_DISTRIBUTION),
    logScale(false)
{
    ui->setupUi(this);
    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->setCanvasBackground(QColor(255,255,255));

    configure_curve(&mainCurve, plotArea, "Bending", QColor(50, 50, 200));
    mainCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);
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

    QComboBox * plotVariantsComboBox = findChild<QComboBox*>("plotVariantsComboBox");
    for(int i = 0; i < _PV_COUNT; ++i)
    {
        plotVariantsComboBox->addItem(plot_captions[i], i);
    }
    plotVariantsComboBox->setCurrentIndex(plotVariant);

    initializing = false;

    refreshPlot();
}

Widget::~Widget()
{
    delete mainCurve;
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
    findChild<QLineEdit*>("fermiLevelLineEdit")->setText(QString::number(model->get_fermi_level_eV(), 'f', 6));

    double Ev = 0;
    double Ec = Ev + model->get_Eg_eV();

    switch(plotVariant)
    {
    case PV_FERMI_DISTRIBUTION:
        model->get_fermi_data_eV(plot_data);
        mainCurve->setSamples(plot_data.xs, plot_data.ys);
        break;

    case PV_FERMI_LEVEL:
        model->get_fermi_level_data_eV(plot_data);
        mainCurve->setSamples(plot_data.xs, plot_data.ys);
        break;

    case PV_NA:
        if(logScale)
            mainCurve->setSamples(model->get_Na_log_data().xs, model->get_Na_log_data().ys);
        else
            mainCurve->setSamples(model->get_Na_data().xs, model->get_Na_data().ys);
        break;

    case PV_ND:
        if(logScale)
            mainCurve->setSamples(model->get_Nd_log_data().xs, model->get_Nd_log_data().ys);
        else
            mainCurve->setSamples(model->get_Nd_data().xs, model->get_Nd_data().ys);
        break;

    case PV_N:
        if(logScale)
            mainCurve->setSamples(model->get_n_log_data().xs, model->get_n_log_data().ys);
        else
            mainCurve->setSamples(model->get_n_data().xs, model->get_n_data().ys);
        break;

    case PV_P:
        if(logScale)
            mainCurve->setSamples(model->get_p_log_data().xs, model->get_p_log_data().ys);
        else
            mainCurve->setSamples(model->get_p_data().xs, model->get_p_data().ys);
        break;
    }

    double xmin = mainCurve->minXValue();
    double xmax = mainCurve->maxXValue();
    set_level(EvCurve, Ev, xmin, xmax);
    set_level(EcCurve, Ec, xmin, xmax);
    set_level(EaCurve, Ev + model->get_Ea_eV(), xmin, xmax);
    set_level(EdCurve, Ec - model->get_Ed_eV(), xmin, xmax);
    set_level(fermiLevelCurve, Ev + model->get_fermi_level_eV(), xmin, xmax);

    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->replot();
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
    findChild<QDoubleSpinBox*>("TminSpinner")->setValue(model->get_Tmin());
    findChild<QDoubleSpinBox*>("TmaxSpinner")->setValue(model->get_Tmax());
    findChild<QDoubleSpinBox*>("TstepSpinner")->setValue(model->get_Tstep());
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

void Widget::update_Na()
{
    double Na_value = density_slider_value(findChild<QSlider*>("NaSlider"));
    findChild<QLineEdit*>("NaLineEdit")->setText(format_density(Na_value));

    if(acceptorEnabled)
    {
        model->set_density_acceptor(Na_value);
    }
    else
    {
        model->set_density_acceptor(0);
    }
}

void Widget::update_Nd()
{
    double Nd_value = density_slider_value(findChild<QSlider*>("NdSlider"));
    findChild<QLineEdit*>("NdLineEdit")->setText(format_density(Nd_value));

    if(donorEnabled)
    {

        model->set_density_donor(Nd_value);
    }
    else
    {
        model->set_density_donor(0);
    }
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
    update_Na();
    if(acceptorEnabled)
    {
        if(plotVariant == PV_FERMI_DISTRIBUTION || plotVariant == PV_FERMI_LEVEL)
            EaCurve->attach(findChild<QwtPlot*>("plotArea"));
    }
    else
    {
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
    update_Nd();
    if(donorEnabled)
    {
        if(plotVariant == PV_FERMI_DISTRIBUTION || plotVariant == PV_FERMI_LEVEL)
            EdCurve->attach(findChild<QwtPlot*>("plotArea"));
    }
    else
    {
        EdCurve->detach();
    }

    findChild<QDoubleSpinBox*>("EdSpinner")->setEnabled(donorEnabled);
    findChild<QSlider*>("NdSlider")->setEnabled(donorEnabled);
    findChild<QLineEdit*>("NdLineEdit")->setEnabled(donorEnabled);
    refreshPlot();
}

void Widget::on_TminSpinner_valueChanged(double value)
{
    model->set_Tmin(value);
    refreshPlot();
}

void Widget::on_TmaxSpinner_valueChanged(double value)
{
    model->set_Tmax(value);
    refreshPlot();
}

void Widget::on_TstepSpinner_valueChanged(double value)
{
    model->set_Tstep(value);
    refreshPlot();
}

void Widget::on_plotVariantsComboBox_currentIndexChanged(int index)
{
    plotVariant = index;
    reattach_level_curves();
    refreshPlot();
}

void Widget::reattach_level_curves()
{
    QwtPlot * plotArea = findChild<QwtPlot*>("plotArea");
    if(plotVariant == PV_FERMI_DISTRIBUTION || plotVariant == PV_FERMI_LEVEL)
    {
        EcCurve->attach(plotArea);
        EvCurve->attach(plotArea);
        if(acceptorEnabled)
            EaCurve->attach(plotArea);
        if(donorEnabled)
            EdCurve->attach(plotArea);
    }
    else
    {
        EcCurve->detach();
        EvCurve->detach();
        EaCurve->detach();
        EdCurve->detach();
    }

    if(plotVariant == PV_FERMI_DISTRIBUTION)
    {
        fermiLevelCurve->attach(plotArea);
    }
    else
    {
        fermiLevelCurve->detach();
    }
}

void Widget::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot to File"),
                                                    "data.csv",
                                                    tr("Comma Separated Values (*.csv)"));
    DataSeries tempData;
    const DataSeries * saveData = &tempData;

    switch(plotVariant)
    {
    case PV_FERMI_DISTRIBUTION:
        model->get_fermi_data_eV(tempData);
        break;

    case PV_FERMI_LEVEL:
        model->get_fermi_level_data_eV(tempData);
        break;

    case PV_NA:
        if(logScale)
            saveData = &model->get_Na_log_data();
        else
            saveData = &model->get_Na_data();
        break;

    case PV_ND:
        if(logScale)
            saveData = &model->get_Nd_log_data();
        else
            saveData = &model->get_Nd_data();
        break;

    case PV_N:
        if(logScale)
            saveData = &model->get_n_log_data();
        else
            saveData = &model->get_n_data();
        break;

    case PV_P:
        if(logScale)
            saveData = &model->get_p_log_data();
        else
            saveData = &model->get_p_data();
        break;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream outStream(&file);
    for(int i = 0; i < saveData->size(); ++i)
    {
        outStream << saveData->xs[i] << ", " << saveData->ys[i] << '\n';
    }
}

void Widget::on_logScaleCheckBox_stateChanged(int state)
{
    logScale = (state == Qt::Checked);
    refreshPlot();
}
