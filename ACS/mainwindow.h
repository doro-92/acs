#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include "DataPlot/qcustomplot.h"
#include "EXT/ext.h"
#include "DBControl/dbcontrol.h"
#include "device/device_plc_f5a.h"



namespace Ui {
class MainWindow;

}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
    void InitCore();
    void InitPlot();
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

    QPixmap myPaint(QRgb color);
    void SetColorLabel(QLabel *lb, bool flg);
private slots:
  void TestTimerTick();
  void timerTick();
  void titleDoubleClick(QMouseEvent *event, QCPPlotTitle *title);
  void axisLabelDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
  void legendDoubleClick(QCPLegend* legend, QCPAbstractLegendItem* item);
  void selectionChanged();
  void mousePress(QMouseEvent* event);
  void mouseRelease(QMouseEvent* event);
  void mouseWheel();
  void addGraph();
  void DrawMultiGraph(quint32 id);
//  void addRandomGraph();
  void contextMenuRequest(QPoint pos);
  void moveLegend();
  void graphClicked(QCPAbstractPlottable *plottable);
  
  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

  void on_pushButton_3_clicked();

  void on_pushButton_6_clicked();

  void on_pushButton_7_clicked();

  void on_pushButton_8_clicked();

  void on_pushButton_5_clicked();

  void on_pushButton_4_clicked();

  void on_pushButton_9_clicked();

private:
  Ui::MainWindow *ui;
  QPoint startPoint;

  void InitTimer();

};

#endif // MAINWINDOW_H
