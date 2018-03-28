#ifndef EXT_H
#define EXT_H

#include <QQueue>

////////////////////////////////
//глобальные типы структуры
//структура для семафоров БД
typedef struct DBSemaphores
{
    bool QueueWr;
    //!флаг чтения из очереди DataFlow
    bool QueueRd;

    //!флаг наличия запроса выборки из БД (запрос в ожидании)
     bool WrQueueDB;
    //!флаг записи в очередь на вывод из БД
    bool DBOutputWR;
    //!флаг чтения из очереди на вывод из БД (пишет кор)
    bool DBOutputRd;

    DBSemaphores()
    {
        QueueWr=false;
        QueueRd=false;
        WrQueueDB=false;
        DBOutputWR=false;
        DBOutputRd=false;
    }
}DBSemaphores;

//структура потока данных от уст-в
typedef struct structDataFlow
{
    quint32 numDevice; //номер уст-ва
    quint32 numParam; //номер параметра
    QString time;
    QString data;
}structDataFlow;

/*
typedef struct structDataConfig
{
    quint32 numDevice; //номер уст-ва
    quint32 numParam; //номер параметра
    QString Coeff; //коэффициент
    QString TxtMeasure; //тектовое обозначение единицы измерения
}structDataConfig;
*/

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
////////////////////!!! coefficient возможно надо double? /////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!
typedef struct ConfigStruct //структура "Конфигурация", описывающая параметры, поступающие от устройства
{
    qint16 param_number; //номер параметра,
    QString coefficient; //коэффициент на который нужно домножать,
    QString unit; //единица измерения,
    QString param_name; //название параметра (например "ru.00 - состояние инвертора" )
    qint32 param_adress; //адрес параметров ПЧ
    bool type; //тип данных, 0 - int, 1- double
}ConfigStruct;


typedef struct structCurrentDataConfig
{
    quint32 ID; //id парметра
    quint32 numDevice; //номер уст-ва
    qint16 param_number; //номер параметра
    QString param_name; //название параметра (например "ru.00 - состояние инвертора" )
    QString unit; //единица измерения,
    QString coefficient;//коэффициент на который нужно домножать,
    bool type; //тип данных, 0 - int, 1- double
}structCurrentDataConfig;

typedef struct structNewDataFlow
{
    quint32 id;
    QString time;
    QString data;
}structNewDataFlow;

typedef struct structSelectFromDB
{
    bool ok; //заглушка, если не надо ничего =false
    quint32 id; //ид из CurrentConfig на параметр
    QString time;// время, если 0 то весь кусок, или от времени нужного
}structSelectFromDB;
#endif // EXT_H
