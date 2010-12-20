#include "model.h"
#include <cmath>

namespace
{
    // Fundametnal
    const double k = 1.380662e-16; // erg * K
    const double default_eff_density = 2.51e+19; // cm^-3

    // Silicon:
    const double silicon_Eg = electron_volt_to_erg(1.12); // erg
    const double silicon_mc = 0.36 * ELECTRON_MASS; // gramm
    const double silicon_mv = 0.81 * ELECTRON_MASS; // gramm
    const double silicon_permittivity = 12; // <no unit>

    // Default:
    const double default_T = 300; // K
    const double default_density_admixture = 1e17; // cm^-3
    const double default_E_admixture = electron_volt_to_erg(0.045); // erg
    const double default_surface_potential = 0.1; // V
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
    density_acceptor = default_density_admixture;
    density_donor = default_density_admixture;
}

void Model::set_others_default()
{
    T = default_T;
    surface_potential = default_surface_potential;
}

void Model::fill_data()
{
    compute_neutral_fermi_level();

    // TODO: real data

    data.xs.clear();
    data.ys.clear();

    for(int i = 0; i < 500; ++i)
    {
        double x = i/100.;

        data.xs.push_back(x);
        data.ys.push_back(sin(x)*T);
    }
}

double eff_density(double m, double T)
{
    return default_eff_density * pow((m/ELECTRON_MASS), (3./2.)) * pow((T/300), (3./2.));
}

void Model::compute_eff_dencities()
{
    eff_density_c = eff_density(mc, T);
    eff_density_v = eff_density(mv, T);
}

double Model::fermi(double energy, double fermi_level)
{
    return exp((fermi_level - energy)/(k * T));
}

double Model::density_n(double energy, double fermi_level)
{
    return eff_density_c * fermi(energy, fermi_level);
}

double Model::density_p(double energy, double fermi_level)
{
    return eff_density_v * (1 - fermi(energy, fermi_level));
}

double Model::density_donor_p(double energy, double fermi_level)
{
    return density_donor * (1 - fermi(energy, fermi_level));
}

double Model::density_acceptor_n(double energy, double fermi_level)
{
    return density_acceptor * fermi(energy, fermi_level);
}

double Model::neutrality_function(double fermi_level)
{
    return density_donor_p(Eg - Ed, fermi_level) + density_p(0, fermi_level)
          -density_n(Eg, fermi_level) - density_acceptor_n(Ea, fermi_level);
}

void Model::compute_neutral_fermi_level()
{
    double a = 0.0;
    double b = Eg;
    double precision = 1e-6*std::min(Ea, Ed);

    if( equal(0, neutrality_function(a)) )
    {
        neutral_fermi_level = a;
        return;
    }
    if( equal(0, neutrality_function(b)) )
    {
        neutral_fermi_level = b;
        return;
    }

    Q_ASSERT_X(neutrality_function(a)*neutrality_function(b) < 0,
               "Model::compute_neutral_fermi_level", "bisection failed: f(a)*f(b) > 0");

    while(b - a > precision)
    {
        double c = (a + b)/2;

        if(neutrality_function(a)*neutrality_function(c) < 0)
            b = c;
        else
            a = c;
    }

    neutral_fermi_level = (a + b)/2;

    double expected_precision = 1e-3*std::min(density_acceptor, density_donor);
    Q_ASSERT_X( equal(0, neutrality_function(neutral_fermi_level), expected_precision),
                "Model::compute_neutral_fermi_level", "bisection gave a wrong result: f(c) != 0");
}
