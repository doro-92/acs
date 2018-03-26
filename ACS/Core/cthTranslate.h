#ifndef CTHTRANSLATE_H
#define CTHTRANSLATE_H

#include <QObject>
#include <QEventLoop>
#include <QThread>
#include <QQueue>
#include <QVector>
#include <QTimer>
#include "ext/ext.h"

//интервал коммита
#define Timer_interval 1000
//сон потока если нет работы (мс)
#define sSleepMS 1

//поток трасляции данных
class cthTranslate: public QObject
{
    Q_OBJECT

public:
    cthTranslate(QVector <QQueue <structDataFlow> *> *DataFlow, QQueue<structDataFlow> *DataFlowToDB, QVector <bool> *vecFlgQueueWr, DBSemaphores *sDBSem);

public slots:
    void s_OnTimer(void);
    void run();
    void StopWork();

private:
    void ShiftElements(quint32 &i);
    QTimer *pTimer;

    QVector <QQueue <structDataFlow> *> *DataFlow;
    QQueue <structDataFlow> *DataFlowToDB;
    QVector <bool> *vecFlgQueueWr;
    DBSemaphores *sDBSem;

    bool Stop;
    bool TimeoutToWrite;
};

#endif // CTHTRANSLATE_H
