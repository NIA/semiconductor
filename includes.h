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
