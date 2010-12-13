#include "model.h"
#include <cmath>

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
