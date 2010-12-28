#pragma once
#include "includes.h"
#include <QVector>

struct DataSeries
{
    QVector<double> xs;
    QVector<double> ys;

    int size() const
    {
        Q_ASSERT(xs.size() == ys.size());
        return xs.size();
    }

    void clear()
    {
        xs.clear();
        ys.clear();
    }

    void push_back(double x, double y)
    {
        xs.push_back(x);
        ys.push_back(y);
    }
};

const double electron_mass = 9.1093e-28; // mass of electron, gramm

class Model
{
private:
    DataSeries fermi_data; // erg(1)
    DataSeries fermi_level_data; // erg(1/kT)
    DataSeries Na_data; // Na(1/kT)
    DataSeries Nd_data; // Na(1/kT)
    DataSeries Na_log_data; // log Na(1/kT)
    DataSeries Nd_log_data; // log Na(1/kT)

    // Parameters

    double Eg; // erg
    double mc; // gramm
    double mv; // gramm
    double permittivity; // <no unit>

    double Tcurrent; // K
    double Tmin; // K
    double Tmax; // K
    double Tstep; // K

    double Ea; // erg
    double Ed; // erg
    double density_acceptor; // cm^-3
    double density_donor; // cm^-3

    void compute_eff_dencities(double T);
    double eff_density_c; // cm^-3
    double eff_density_v; // cm^-3

    double total_surface_charge; // CGS/cm^2
    double min_difference; // CGS/cm^2
    double surface_field; // CGS

    // exp(energy_difference/kT)
    double energy_exp(double energy_difference, double T);
    // fermi distribution
    double fermi(double energy, double fermi_level, double T); // <no unit>

    double density_n(double fermi_level, double T);
    double density_p(double fermi_level, double T);

    double density_donor_p(double fermi_level, double T);
    double density_acceptor_n(double fermi_level, double T);

    double charge_density(double fermi_level /*erg*/, double T /*K*/); // CGS/cm^3

    double compute_neutral_fermi_level(double T);
    double neutral_fermi_level; // erg

    void compute_fermi_distribution(double Estep /*erg*/);
    void compute_dependences();

public:
    Model();

    void set_silicon();
    void set_admixtures_default();
    void set_others_default();

    void fill_data();
    const DataSeries & get_fermi_data_erg() const { return fermi_data; }
    void get_fermi_data_eV(/*out*/ DataSeries & eV_data) const;

    void get_fermi_level_data_eV(/*out*/ DataSeries & eV_data) const;
    const DataSeries & get_Na_data() const { return Na_data; }
    const DataSeries & get_Nd_data() const { return Nd_data; }
    const DataSeries & get_Na_log_data() const { return Na_log_data; }
    const DataSeries & get_Nd_log_data() const { return Nd_log_data; }

    double get_fermi_level_erg() { return neutral_fermi_level; }
    double get_fermi_level_eV() { return erg_to_electron_volt(neutral_fermi_level); }

    double get_Eg_erg() { return Eg; } // erg
    double get_Eg_eV() { return erg_to_electron_volt(Eg); } // eV
    void set_Eg_erg(double value) { Eg = value; } // erg
    void set_Eg_eV(double value) { Eg = electron_volt_to_erg(value); } // eV

    double get_mc_gramm() { return mc; } // gramm
    double get_mc_m0() { return mc/electron_mass; } // m0
    void set_mc_gramm(double value) { mc = value; } // gramm
    void set_mc_m0(double value) { mc =  value * electron_mass; } // m0

    double get_mv_gramm() { return mv; } // gramm
    double get_mv_m0() { return mv/electron_mass; } // m0
    void set_mv_gramm(double value) { mv = value; } // gramm
    void set_mv_m0(double value) { mv =  value * electron_mass; } // m0

    double get_permittivity() { return permittivity; } // <no unit>
    void set_permittivity(double value) { permittivity = value; } // <no unit>

    double get_T() { return Tcurrent; } // K
    void set_T(double value) { Tcurrent = value; } // K
    double get_Tmin() { return Tmin; } // K
    void set_Tmin(double value) { Tmin = value; } // K
    double get_Tmax() { return Tmax; } // K
    void set_Tmax(double value) { Tmax = value; } // K
    double get_Tstep() { return Tstep; } // K
    void set_Tstep(double value) { Tstep = value; } // K

    double get_Ea_erg() { return Ea; } // erg
    double get_Ea_eV() { return erg_to_electron_volt(Ea); } // eV
    void set_Ea_erg(double value) { Ea = value; } // erg
    void set_Ea_eV(double value) { Ea = electron_volt_to_erg(value); } // eV

    double get_Ed_erg() { return Ed; } // erg
    double get_Ed_eV() { return erg_to_electron_volt(Ed); } // eV
    void set_Ed_erg(double value) { Ed = value; } // erg
    void set_Ed_eV(double value) { Ed = electron_volt_to_erg(value); } // eV

    double get_density_acceptor() { return density_acceptor; } // cm^-3
    void set_density_acceptor(double value) { density_acceptor = value; } // cm^-3

    double get_density_donor() { return density_donor; } // cm^-3
    void set_density_donor(double value) { density_donor = value; } // cm^-3
};
