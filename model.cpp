#include "model.h"
#include <cmath>
#include <limits>

namespace
{
    // Fundametnal
    const double k = 1.380662e-16; // erg * K
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
    const double default_density_admixture = 1e17; // cm^-3
    const double default_E_admixture = electron_volt_to_erg(0.045); // erg
    const double default_surface_potential = volt_to_cgs(0.1); // CGS

    // Computation:
    const double electric_field_min = -1e4;
    const double electric_field_max = 1e4;
    const double electric_field_step = 100;
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
    //solve_potential_equation(300, 3e-6, 1e-9);
    do_shooting(3e-6, 5e-9);
}

void Model::get_bending_data_eV(/*out*/ DataSeries & eV_data) const
{
    eV_data.clear();

    for(int i = 0; i < bending_data.size(); ++i)
    {
        eV_data.push_back(bending_data.xs[i], erg_to_electron_volt(bending_data.ys[i]));
    }
}

double eff_density(double m, double T)
{
    return default_eff_density * pow((m/electron_mass), (3./2.)) * pow((T/300), (3./2.));
}

void Model::compute_eff_dencities()
{
    eff_density_c = eff_density(mc, T);
    eff_density_v = eff_density(mv, T);
}

double Model::energy_exp(double energy_difference)
{
    return exp(energy_difference/(k * T));
}

double Model::fermi(double energy, double fermi_level)
{
    return 1/(1 + energy_exp(energy - fermi_level));
}

double Model::density_n(double energy, double fermi_level)
{
    return eff_density_c * energy_exp(fermi_level - energy);
}

double Model::density_p(double energy, double fermi_level)
{
    return eff_density_v * energy_exp(energy - fermi_level);
}

double Model::density_donor_p(double energy, double fermi_level)
{
    return density_donor * (1 - fermi(energy, fermi_level));
}

double Model::density_acceptor_n(double energy, double fermi_level)
{
    return density_acceptor * fermi(energy, fermi_level);
}

double Model::charge_density(double fermi_level /*erg*/, double zone_bending /*erg*/)
{
    return elementary_charge*( density_p(0 + zone_bending, fermi_level)
                             + density_donor_p(Eg - Ed + zone_bending, fermi_level)
                             - density_n(Eg + zone_bending, fermi_level)
                             - density_acceptor_n(Ea + zone_bending, fermi_level) );
}

void Model::compute_neutral_fermi_level()
{
    compute_eff_dencities();

    double a = -Eg;
    double b = 2*Eg;
    double precision = 1e-6*std::min(Ea, Ed);

    if( equal(0, charge_density(a)) )
    {
        neutral_fermi_level = a;
        return;
    }
    if( equal(0, charge_density(b)) )
    {
        neutral_fermi_level = b;
        return;
    }

    Q_ASSERT_X(charge_density(a)*charge_density(b) < 0,
               "Model::compute_neutral_fermi_level", "bisection failed: f(a)*f(b) > 0");

    while(b - a > precision)
    {
        double c = (a + b)/2;

        if(charge_density(a)*charge_density(c) < 0)
            b = c;
        else
            a = c;
    }

    neutral_fermi_level = (a + b)/2;
}

void Model::solve_potential_equation(double surface_electric_field /*CGS*/, double xmax /*cm*/, double xstep /*cm*/)
{
    bending_data.clear();

    double x = 0;
    double potential = surface_potential;
    double electric_field = surface_electric_field;
    total_surface_charge = 0;

    bending_data.push_back(x, electron_charge*potential);

    while(x < xmax)
    {
        double cur_charge_density = charge_density(neutral_fermi_level, electron_charge*potential);
        x += xstep;
        electric_field += -4*M_PI/permittivity*cur_charge_density*xstep;
        potential += - electric_field*xstep;
        total_surface_charge += cur_charge_density*xstep;

        bending_data.push_back(x, electron_charge*potential);
    }
}

void Model::do_shooting(double xmax /*cm*/, double xstep/*cm*/)
{
    double field = electric_field_min;

    min_difference = MAX_DOUBLE;
    double min_diff_field = 0;

    while(field < electric_field_max)
    {
        solve_potential_equation(field, xmax, xstep);
        double difference = total_surface_charge - permittivity*field/2/M_PI;
        if(abs(difference) < min_difference)
        {
            min_difference = abs(difference);
            min_diff_field = field;
        }

        field += electric_field_step;
    }

    surface_field = min_diff_field;

    solve_potential_equation(min_diff_field, xmax, xstep);
}
