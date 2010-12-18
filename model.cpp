#include "model.h"
#include <cmath>

namespace
{
    // Fundametnal
    const double m0 = 9.1093e-28; // gramm
    const double k = 1.380662e-16; // erg * K
    const double default_eff_density = 2.51e+19; // cm^-3

    // Silicon:
    const double silicon_Eg = electron_volt_to_erg(1.12); // erg
    const double silicon_mc = 0.36 * m0; // gramm
    const double silicon_mv = 0.81 * m0; // gramm
    const double silicon_permittivity = 12; // <no unit>

    // Default:
    const double default_T = 300; // K
    const double default_density_admixture = 1e17; // cm^-3
    const double default_E_admixture = electron_volt_to_erg(0.045); // erg
}

Model::Model()
    : T(default_T),
      Ea(default_E_admixture),
      Ed(default_E_admixture),
      density_acceptor(default_density_admixture),
      density_donor(default_density_admixture)
{
    set_silicon();
}

void Model::set_silicon()
{
    Eg = silicon_Eg;
    mc = silicon_mc;
    mv = silicon_mv;
    permittivity = silicon_permittivity;
}

void Model::fill_data()
{
    data.xs.clear();
    data.ys.clear();

    for(int i = 0; i < 500; ++i)
    {
        double x = i/100.;

        data.xs.push_back(x);
        data.ys.push_back(sin(x));
    }
}

double eff_density(double m, double T)
{
    return default_eff_density * pow((m/m0), (3./2.)) * pow((T/300), (3./2.));
}

void Model::compute_eff_dencities()
{
    eff_density_c = eff_density(mc, T);
    eff_density_v = eff_density(mv, T);
}

double Model::fermi(double energy)
{
    return exp((fermi_level - energy)/(k * T));
}

double Model::density_n(double energy)
{
    return eff_density_c * fermi(energy);
}

double Model::density_p(double energy)
{
    return eff_density_v * (1 - fermi(energy));
}

double Model::density_donor_p(double energy)
{
    return density_donor * (1 - fermi(energy));
}

double Model::density_acceptor_n(double energy)
{
    return density_acceptor * fermi(energy);
}
