#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QTimer>
#include "Core/Core.h"



namespace Ui {
class MainWindow;

}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
    void InitCore();
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

    QPixmap myPaint(QRgb color);
    void SetColorLabel(QLabel *lb, bool flg);

private slots:
  void TestTimerTick();
  
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
  Core *oCore;

  QTimer *timer;
  QTimer *timer2;



};

#endif // MAINWINDOW_H
