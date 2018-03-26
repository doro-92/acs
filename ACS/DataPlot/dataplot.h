#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <qvector.h>

typedef struct sDataPlot
{
  QVector <double> x;
  QVector <double> y;
}sDataPlot;

class DataPlot
{
public:
    DataPlot();
    QVector <sDataPlot> vecPlots;
};

#endif // DATAPLOT_H
