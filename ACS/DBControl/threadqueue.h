#ifndef THREADQUEUE_H
#define THREADQUEUE_H

#include <QtSql>
#include <QThread>
#include <QQueue>
#include <QObject>
#include "ext/ext.h"
#include "DBControl/common.h"

//интервал коммита
#define Timer_interval 1000
//сон потока если нет работы (мс)
#define SleepMS 1

//поток записи в бд
class ThreadQueue : public QObject
{
Q_OBJECT
public:
    ThreadQueue(QSqlDatabase *sdb_u, QQueue <structDataFlow> *DataFlowToDB, QQueue <structNewDataFlow> *DataFlowFromDB, DBSemaphores *sDBSem);

   // bool Busy;
    QString Err;
    void GetDBData(qint32 &id, QString &time);
    void SuspendThread(bool flgSuspend);
    bool GetSuspendStatus();
    bool FlushNewConfig(QString &currentID);


    qint32 GetCurrentIdFromNumParam(const QString &numDevice, const qint16 &numParam);
    bool GetCurrentNumParamFromId(const quint32 id, QString &numDevice, quint32 &numParam);

signals:
    void SendError(structDBErrors stDBErr);

public slots:
    void s_OnTimer(void);
    void run();

private:
    bool GetDataFromDB(const qint32 &id, const QString &time);
    //флаг приостановки
    bool cSuspend;
    //флаг что приостановились
    bool Suspend;

    bool TimerSkip;
    bool wait;
    qint32 cnt;
    QSqlDatabase *sdb;

    QQueue <structDataFlow> *DataFlowToDB;
    QQueue <structNewDataFlow> *DataFlowFromDB;
    DBSemaphores *sDBSem;
    //текущие настройки конфига, для преобразования значений от устройств (уже с id)
    QVector <structCurrentDataConfig> *CurrentSessionConfig;

    QTimer *pTimer;

    //структура запроса из БД
    qint32 id;
    QString time;


};


#endif // THREADQUEUE_H
