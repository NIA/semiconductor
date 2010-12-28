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
    DataSeries plot_data;
    QwtPlotCurve * mainCurve;
    QwtPlotCurve * EvCurve;
    QwtPlotCurve * EcCurve;
    QwtPlotCurve * EdCurve;
    QwtPlotCurve * EaCurve;
    QwtPlotCurve * fermiLevelCurve;
    bool initializing;

    bool acceptorEnabled;
    bool donorEnabled;

    int plotVariant;

    void reattach_level_curves();
    void refreshPlot();

    void copySiliconFromModel();
    void copyAdmixturesDefaultFromModel();
    void copyOthersDefaultFromModel();

    void update_Na();
    void update_Nd();

private slots:
    void on_pushButton_clicked();
    void on_plotVariantsComboBox_currentIndexChanged(int index);
    void on_TstepSpinner_valueChanged(double );
    void on_TmaxSpinner_valueChanged(double );
    void on_TminSpinner_valueChanged(double );
    void on_donorCheckBox_stateChanged(int );
    void on_acceptorCheckBox_stateChanged(int );
    void on_NdSlider_valueChanged(int value);
    void on_NaSlider_valueChanged(int value);
    void on_TSpinner_valueChanged(double );
    void on_EdSpinner_valueChanged(double );
    void on_EaSpinner_valueChanged(double );
    void on_permittivitySpinner_valueChanged(double );
    void on_mvSpinner_valueChanged(double );
    void on_mcSpinner_valueChanged(double );
    void on_EgSpinner_valueChanged(double );
    void on_othersDefaultButton_clicked();
    void on_admixturesDefaultButton_clicked();
    void on_siliconButton_clicked();
};
