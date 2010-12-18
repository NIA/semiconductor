#pragma once
#include "includes.h"
#include <QVector>

struct DataSeries
{
    QVector<double> xs;
    QVector<double> ys;

    int size()
    {
        Q_ASSERT(xs.size() == ys.size());
        return xs.size();
    }
};

class Model
{
private:
    DataSeries data;

    // Parameters

    double Eg; // erg
    double mc; // gramm
    double mv; // gramm
    double permittivity; // <no unit>

    double T; // K

    double Ea; // erg
    double Ed; // erg
    double density_acceptor; // cm^-3
    double density_donor; // cm^-3

    void set_silicon();

    double neutrality_function(double fermi_level);

    void compute_neutral_fermi_level();
    double neutral_fermi_level; // erg

    // fermi distribution
    double fermi(double energy, double fermi_level); // <no unit>

    void compute_eff_dencities();
    double eff_density_c; // cm^-3
    double eff_density_v; // cm^-3

    double density_n(double energy, double fermi_level);
    double density_p(double energy, double fermi_level);

    double density_donor_p(double energy, double fermi_level);
    double density_acceptor_n(double energy, double fermi_level);

public:
    Model();
    void fill_data();
    const DataSeries & get_data() const { return data; }
};
