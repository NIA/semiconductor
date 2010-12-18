#include "model.h"
#include <cmath>

namespace
{
    // CGS
    const double m0 = 9.1093e-28; // gramm
    const double k = 1.380662e-16; // erg * K
    const double default_eff_density = 2.51e+19; // cm^-3
}

Model::Model()
{
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
