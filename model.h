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
    DataSeries bending_data; // erg(cm)

    // Parameters

    double Eg; // erg
    double mc; // gramm
    double mv; // gramm
    double permittivity; // <no unit>

    double T; // K
    double surface_potential; // V

    double Ea; // erg
    double Ed; // erg
    double density_acceptor; // cm^-3
    double density_donor; // cm^-3

    void compute_eff_dencities();
    double eff_density_c; // cm^-3
    double eff_density_v; // cm^-3

    // exp(energy_difference/kT)
    double energy_exp(double energy_difference);
    // fermi distribution
    double fermi(double energy, double fermi_level); // <no unit>

    double density_n(double energy, double fermi_level);
    double density_p(double energy, double fermi_level);

    double density_donor_p(double energy, double fermi_level);
    double density_acceptor_n(double energy, double fermi_level);

    double charge_density(double fermi_level /*erg*/, double zone_bending = 0 /*erg*/);

    void compute_neutral_fermi_level();
    double neutral_fermi_level; // erg

    void solve_potential_equation(double surface_electric_field /*CGS*/, double xmax /*cm*/, double xstep /*cm*/);

public:
    Model();

    void set_silicon();
    void set_admixtures_default();
    void set_others_default();

    void fill_data();
    const DataSeries & get_bending_data_erg() const { return bending_data; }
    void get_bending_data_eV(/*out*/ DataSeries & eV_data) const;

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

    double get_T() { return T; } // K
    void set_T(double value) { T = value; } // K

    double get_surface_potential_CGS() { return surface_potential; }
    void set_surface_potential_CGS(double value) { surface_potential = value; }
    double get_surface_potential_volt() { return cgs_to_volt(surface_potential); }
    void set_surface_potential_volt(double value) { surface_potential = volt_to_cgs(value); }

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
