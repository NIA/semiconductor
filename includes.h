#pragma once

#include <QtGlobal> // for Q_ASSERT
#include <cmath>

const int DEFAULT_PRECISION = 1e-14;
inline bool equal(double a, double b, double precision = DEFAULT_PRECISION)
{
    return std::abs(a - b) < precision;
}

const double ELECTRON_VOLT_IN_ERGS = 1.60218e-12;
inline double electron_volt_to_erg(double electron_volt)
{
    return electron_volt*ELECTRON_VOLT_IN_ERGS;
}
inline double erg_to_electron_volt(double erg)
{
    return erg/ELECTRON_VOLT_IN_ERGS;
}

const double VOLT_IN_CGS = 1.0/300;
inline double volt_to_cgs(double volt)
{
    return volt*VOLT_IN_CGS;
}
inline double cgs_to_volt(double cgs)
{
    return cgs/VOLT_IN_CGS;
}
