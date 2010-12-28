#include "model.h"
#include <cmath>
#include <limits>

namespace
{
    // Fundametnal
    const double default_eff_density = 2.51e+19; // cm^-3
    const double elementary_charge = 4.80320427e-10; // CGS
    const double electron_charge = -elementary_charge; // CGS
    const double MAX_DOUBLE = std::numeric_limits<double>::max();

    // Silicon:
    const double silicon_Eg = electron_volt_to_erg(1.12); // erg
    const double silicon_mc = 0.36 * electron_mass; // gramme
    const double silicon_mv = 0.81 * electron_mass; // gramme
    const double silicon_permittivity = 12; // <no unit>

    // Default:
    const double default_T = 300; // K
    const double default_Tmin = 50; // K
    const double default_Tmax = 1500; // K
    const double default_Tstep = 1; // K
    const double default_density_donor = 1e17; // cm^-3
    const double default_density_acceptor = 5e16; // cm^-3
    const double default_E_admixture = electron_volt_to_erg(0.045); // erg
    const double default_surface_potential = volt_to_cgs(0.1); // CGS

    // Computation:
    const double electric_field_min = -1e4;
    const double electric_field_max = 1e4;
    const double electric_field_step = 100;
    const double xmax = 3e-6;
}

Model::Model()
{
    set_silicon();
    set_admixtures_default();
    set_others_default();
}

void Model::set_silicon()
{
    Eg = silicon_Eg;
    mc = silicon_mc;
    mv = silicon_mv;
    permittivity = silicon_permittivity;
}

void Model::set_admixtures_default()
{
    Ea = default_E_admixture;
    Ed = default_E_admixture;
    density_acceptor = default_density_acceptor;
    density_donor = default_density_donor;
}

void Model::set_others_default()
{
    Tcurrent = default_T;
    Tmin = default_Tmin;
    Tmax = default_Tmax;
    Tstep = default_Tstep;
}

void Model::fill_data()
{
    neutral_fermi_level = compute_neutral_fermi_level(Tcurrent);
    compute_fermi_distribution(Eg/500);
    compute_dependences();
}

void Model::get_fermi_data_eV(/*out*/ DataSeries & eV_data) const
{
    eV_data.clear();

    for(int i = 0; i < fermi_data.size(); ++i)
    {
        eV_data.push_back(fermi_data.xs[i], erg_to_electron_volt(fermi_data.ys[i]));
    }
}

void Model::get_fermi_level_data_eV(/*out*/ DataSeries & eV_data) const
{
    eV_data.clear();

    for(int i = 0; i < fermi_level_data.size(); ++i)
    {
        eV_data.push_back(fermi_level_data.xs[i], erg_to_electron_volt(fermi_level_data.ys[i]));
    }
}

double eff_density(double m, double T)
{
    return default_eff_density * pow((m/electron_mass), (3./2.)) * pow((T/300), (3./2.));
}

void Model::compute_eff_dencities(double T)
{
    eff_density_c = eff_density(mc, T);
    eff_density_v = eff_density(mv, T);
}

double Model::energy_exp(double energy_difference, double T)
{
    return exp(energy_difference/(k * T));
}

double Model::fermi(double energy, double fermi_level, double T)
{
    return 1/(1 + energy_exp(energy - fermi_level, T));
}

double Model::density_n(double fermi_level, double T)
{
    return eff_density_c * energy_exp(fermi_level - Eg, T);
}

double Model::density_p(double fermi_level, double T)
{
    return eff_density_v * energy_exp(0 - fermi_level, T);
}

double Model::density_donor_p(double fermi_level, double T)
{
    return density_donor * (1 - fermi(Eg - Ed, fermi_level, T));
}

double Model::density_acceptor_n(double fermi_level, double T)
{
    return density_acceptor * fermi(Ea, fermi_level, T);
}

double Model::charge_density(double fermi_level /*erg*/, double T /*K*/)
{
    return elementary_charge*( density_p(fermi_level, T)
                             + density_donor_p(fermi_level, T)
                             - density_n(fermi_level, T)
                             - density_acceptor_n(fermi_level, T) );
}

double Model::compute_neutral_fermi_level(double T)
{
    compute_eff_dencities(T);

    double a = -Eg;
    double b = 2*Eg;
    double precision = 1e-6*std::min(Ea, Ed);

    if( equal(0, charge_density(a, T)) )
    {
        return a;
    }
    if( equal(0, charge_density(b, T)) )
    {
        return b;
    }

    Q_ASSERT_X(charge_density(a, T)*charge_density(b, T) < 0,
               "Model::compute_neutral_fermi_level", "bisection failed: f(a)*f(b) > 0");

    while(b - a > precision)
    {
        double c = (a + b)/2;

        if(charge_density(a, T)*charge_density(c, T) < 0)
            b = c;
        else
            a = c;
    }

    return (a + b)/2;
}

void Model::compute_fermi_distribution(double Estep /*erg*/)
{
    fermi_data.clear();

    for(double E = 0; E <= Eg; E+= Estep)
    {
        fermi_data.push_back(fermi(E, neutral_fermi_level, Tcurrent), E);
    }
}

void Model::compute_dependences()
{
    fermi_level_data.clear();
    Nd_data.clear();
    Na_data.clear();
    Nd_log_data.clear();
    Na_log_data.clear();
    n_data.clear();
    p_data.clear();
    n_log_data.clear();
    p_log_data.clear();

    double fermi_level;
    double kT_inv;
    for(double T = Tmin; T <= Tmax; T += Tstep)
    {
        fermi_level = compute_neutral_fermi_level(T);
        kT_inv = 1/erg_to_electron_volt(k*T);

        fermi_level_data.push_back(kT_inv, fermi_level);
        Nd_data.push_back(kT_inv, density_donor_p(fermi_level,T));
        Na_data.push_back(kT_inv, density_acceptor_n(fermi_level, T));
        Nd_log_data.push_back(kT_inv, log10(density_donor_p(fermi_level,T)));
        Na_log_data.push_back(kT_inv, log10(density_acceptor_n(fermi_level, T)));
        n_data.push_back(kT_inv, density_n(fermi_level, T));
        p_data.push_back(kT_inv, density_p(fermi_level, T));
        n_log_data.push_back(kT_inv, log10(density_n(fermi_level, T)));
        p_log_data.push_back(kT_inv, log10(density_p(fermi_level, T)));
    }
}

