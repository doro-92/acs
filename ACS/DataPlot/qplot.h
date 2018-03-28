#ifndef QPLOT_H
#define QPLOT_H

#include <QWidget>
#include "qcustomplot.h"
#include "EXT/ext.h"

typedef struct vectorData
{
  QVector <double> T;
  QVector <double> Y;
  QString Name;
}vectorData;

class QPlot : public QWidget
{
    Q_OBJECT
public://сделай еще наверно с боку список, где можно выбрать параметры которые показывать
    explicit QPlot(QWidget *parent = 0/*, QQueue <structNewDataFlow> *DataFlowFromDB*/); //это очередь ответов из БД
    ~QPlot();
    void addGraph(QString name, QVector <double> x, QVector <double> y);

    //заглушки, если бремя переложить на кор, то при масштабировании как из БД куски получать???
    void SetConfig(QVector <structCurrentDataConfig> *CurrentSessionConfig); //см. структуру в ext.h. Для загрузки конфига в твой класс
    //запрос данных. Данные складываются в DataFlowFromDB
    structSelectFromDB GetNextParam(); // вызывается в core на сколько возможно, при каждом вызове должен быть новый параметр или ok=false.
    //придется тебе проверять очередь, извлекать и приводить данные к нужному формату и домнажать на коэффициент
    //все есть в конфиге

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
  /* QQueue <structNewDataFlow> *DataFlowFromDB*/ //хранишь указатель на очередь
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
