#pragma once
#include "includes.h"
#include <QVector>

struct DataSeries
{
    QVector<double> xs;
    QVector<double> ys;

    int size()
    {
        Q_ASSERT(xs.size() == ys.size());
        return xs.size();
    }
};

class Model
{
private:
    DataSeries data;

public:
    Model();

    void fill_data();

    const DataSeries & get_data() const { return data; }
};
