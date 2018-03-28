#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QThread>
#include <QVector>
//#include <QQueue>
#include "ext/ext.h"
#include "Core/cthTranslate.h"

#include "DBControl/dbcontrol.h"
#include "Device/device_plc_f5a.h"
#include "RuleControl/RuleControl.h"
#include "DataPlot/qplot.h"

//класс связи и управления всем
class Core: public QObject
{
    Q_OBJECT
private:
    //количество очередей
    quint32 numDFqueue;
    //очереди данных от уст-в (вектор указателей на очереди)
    QVector <QQueue <structDataFlow> *> vecDataFlow;
    //очередь к БД из очередей DataFlow
    QQueue <structDataFlow> *DataFlowToDB;
    //вывод данных из БД (пока свободен), время и данные
    QQueue <structNewDataFlow> *DataFlowFromDB;

    //вектор флагов записи в очереди от устройств
    QVector <bool> vecFlgQueueWr; //bool *flg=&vecFlgQueueWr[0]; разименовать *flg

    //флаги работы с очередями БД
    DBSemaphores *sDBSem;
    //QVector <bool> vecFlgQueueDB;     //0 - запись, 1- чтение, 2- запись на вывод, 3- чтение на вывод


    //указатели на объекты классов
    //поток обработки очередей
    QThread *objTH;
    //обьект класса обработки очередей
    cthTranslate *objThTranslate;

    //устройства
     device_PLC_F5A *objPLCF5A;

    //основная БД
    DBControl *objDBC;

    //правила
    RuleControl *objRule;

    //график
    QPlot *objQPlot;

public:
    Core(const quint32 &numDFqueue, QWidget *ProgWgt, QWidget *PlotWgt, QPlainTextEdit *plainTextEdit);

    void Parse(QPlainTextEdit *plainTextEdit2)
    {
        objRule->Parse(plainTextEdit2);
    }

    //управление
    //создание или дополнение файла БД
    bool OpenCreateDBFromSerialNumber(QString serialNumber);
    //открыта ли БД
    bool GetStatusOpenDB();
    //закрытие БД
    void CloseDB();
    //закрыта ли БД
    bool GetSuspendStatusDB();
    //запуск тестирования
    bool StartTest();
    //остановка тестирования
    bool StopTest();

    //----информационные методы-------//
    //возвращает состояние очереди уст-ва
    bool GetFlgDataFlow(qint32 i);
    //возвращает структуру флагов ДБ
    DBSemaphores GetFlgDB();
    //количество потоков уст-в
    quint32 GetCountDFQueue();
    //пустой ли поток к БД?
    bool GetIsEmptyDFdb();
    //работает ли поток БД?
    bool GetStatusThDB();

    // тестовые функции класса
    //устанавливает вектор всем потокам
    void TestSetDataFlow(QVector<structDataFlow> *in_vecDataFlow);
    //устанавливает состояние потока уст-ва
    void TestSetFlgDataFlow(quint32 i,bool flg);
    //запустить или остановить поток БД
    void TestControlDBThread();
    QString TestGetLastErrorDB();


    //~Core();
};

#endif // CORE_H
