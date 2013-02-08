#include "widget.h"
#include "ui_widget.h"

#include <qwt_plot.h>
#include <QtWidgets/QtWidgets>

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
}

Widget::Widget(Model * model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    model(model),
    initializing(true),
    acceptorEnabled(false),
    donorEnabled(true),
    plotVariant(PV_FERMI_DISTRIBUTION),
    logYScale(false),
    invertedXScale(true)
{
    ui->setupUi(this);
    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->setCanvasBackground(QColor(255,255,255));

    configure_curve(&mainCurve, plotArea, "", QColor(50, 50, 200));
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

    findChild<QCheckBox*>("logScaleCheckBox")->setChecked(logYScale);
    findChild<QCheckBox*>("invertedScaleCheckBox")->setChecked(invertedXScale);

    QComboBox * plotVariantsComboBox = findChild<QComboBox*>("plotVariantsComboBox");
    const QString plot_captions[_PV_COUNT] =
    {
        tr("Fermi distribution"),
        tr("Fermi level"),
        tr("Charged acceptors"),
        tr("Charged donors"),
        tr("Free electrons"),
        tr("Free holes")
    };
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
    update_plot_data();
    mainCurve->setSamples(plotData.xs, plotData.ys);

    double Ev = 0;
    double Ec = Ev + model->get_Eg_eV();

    double xmin = mainCurve->minXValue();
    double xmax = mainCurve->maxXValue();
    set_level(EvCurve, Ev, xmin, xmax);
    set_level(EcCurve, Ec, xmin, xmax);
    set_level(EaCurve, Ev + model->get_Ea_eV(), xmin, xmax);
    set_level(EdCurve, Ec - model->get_Ed_eV(), xmin, xmax);
    set_level(fermiLevelCurve, Ev + model->get_fermi_level_eV(), xmin, xmax);

    QwtPlot* plotArea = findChild<QwtPlot*>("plotArea");
    plotArea->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plotArea->setAxisTitle(QwtPlot::yLeft, yAxisTitle);
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
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream outStream(&file);
    for(int i = 0; i < plotData.size(); ++i)
    {
        outStream << plotData.xs[i] << ", " << plotData.ys[i] << '\n';
    }
}

void Widget::on_logScaleCheckBox_stateChanged(int state)
{
    logYScale = (state == Qt::Checked);
    refreshPlot();
}

void Widget::on_invertedScaleCheckBox_stateChanged(int state)
{
    invertedXScale = (state == Qt::Checked);
    refreshPlot();
}

void Widget::update_plot_data()
{
    Transformation x_transform = invertedXScale ? transform_T_to_inverted_kT : no_transform;
    Transformation y_transform = logYScale ? log10 : no_transform;

    xAxisTitle = invertedXScale ? tr("1/kT, eV<sup>-1</sup>") : tr("Temperature, K");

    switch(plotVariant)
    {
    case PV_FERMI_DISTRIBUTION:
        // NOTE: x_transform and y_transform are ignored for this plot
        model->get_fermi_data(plotData, no_transform, erg_to_electron_volt);
        xAxisTitle = tr("Fermi distribution function");
        yAxisTitle = tr("Energy, eV");
        break;

    case PV_FERMI_LEVEL:
        // NOTE: y_transform is ignored for this plot
        model->get_fermi_level_data(plotData, x_transform, erg_to_electron_volt);
        yAxisTitle = tr("Energy, eV");
        break;

    case PV_NA:
        model->get_Na_data(plotData, x_transform, y_transform);
        yAxisTitle = logYScale ? tr("log(N<sub>a</sub>), cm<sup>-3</sup>") : tr("N<sub>a</sub>, cm<sup>-3</sup>");
        break;

    case PV_ND:
        model->get_Nd_data(plotData, x_transform, y_transform);
        yAxisTitle = logYScale ? tr("log(N<sub>d</sub>), cm<sup>-3</sup>") : tr("N<sub>d</sub>, cm<sup>-3</sup>");
        break;

    case PV_N:
        model->get_n_data(plotData, x_transform, y_transform);
        yAxisTitle = logYScale ? tr("log(n), cm<sup>-3</sup>") : tr("n, cm<sup>-3</sup>");
        break;

    case PV_P:
        model->get_p_data(plotData, x_transform, y_transform);
        yAxisTitle = logYScale ? tr("log(p), cm<sup>-3</sup>") : tr("p, cm<sup>-3</sup>");
        break;
    }
}
