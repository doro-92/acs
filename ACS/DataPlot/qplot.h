#ifndef QPLOT_H
#define QPLOT_H

#include <QWidget>
#include "qcustomplot.h"

typedef struct vectorData
{
  QVector <double> T;
  QVector <double> Y;
  QString Name;
}vectorData;

class QPlot : public QWidget
{
    Q_OBJECT
public:
    explicit QPlot(QWidget *parent = 0/*= nullptr*/);
    ~QPlot();
    void addGraph(QString name, QVector <double> x, QVector <double> y);

private slots:
  void selectionChanged();
  void mousePress(QMouseEvent* event);
  void mouseRelease(QMouseEvent* event);
  void mouseWheel();
  void addRandomGraph();
  void plusRandomGraph();
  void removeSelectedGraph();
  void removeAllGraphs();
  void contextMenuRequest(QPoint pos);
  void moveLegend();
  void graphClicked(QCPAbstractPlottable *plottable);
  void ShowSeparate();
  void ShowTogether();

private:
  QCustomPlot *cPlot;
  ///*VBox*/QLayout *verticalLayout;
  QPoint startPoint;
  void InitPlot(QWidget *parent);
  void setSeparate(int graphNum);
  void setTogether(int graphNum);
  void ReDrawPlots(int graphNum);
  void setAxisColor(QColor color, int graphNum);
  int getGraphNum();

};

#endif // QPLOT_H
