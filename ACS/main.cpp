#include <QApplication>
#include "mainwindow.h"

//device_PLC_F5A *device1;

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;

  w.InitCore();
  w.show();
  
  return a.exec();
}
