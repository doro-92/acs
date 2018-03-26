#ifndef DB_CONTROL_H
#define DB_CONTROL_H

#include <QtSql>
#include <QThread>
#include <QQueue>
#include <QObject>
#include "ext/ext.h"
#include "DBControl/threadqueue.h"
#include "DBControl/common.h"

//декларируем структуру что бы передавать через сигнал/слот
Q_DECLARE_METATYPE(structDBErrors)


/*все методы класса возвращают LastError или текст ошибки в виде строки*/
class DBControl: public QObject
{
    Q_OBJECT
public:
    DBControl(QQueue <structDataFlow> *DataFlowToDB, QQueue <structNewDataFlow> *DataFlowFromDB, DBSemaphores *sDBSem);
    ~DBControl();

    //структура с флагами ошибок
    structDBErrors DBErrors;
    //сброс ошибок
    void ClearErrors();

    //создаем БД, путь+файл (например, путь+"\\db.sqlite3")
    // параметр rewrite=true перезаписывает файл
    bool CreateDB(QString db_path, bool rewrite);

    //открытие БД, путь+файл (например, путь+"\\db.sqlite3")
    bool OpenDB(QString db_path);

    void CloseDB();

    //создание новой сессии
    bool CreateSession();

    //добавление конфига из каждого устройства
    bool AddConfig(QString uniq_num_dev, QVector <ConfigStruct> *Config);

    //формирование нового конфига из всех новых импортнутых через AddConfig()
    bool FlushNewConfig();


    //управлением потоком записи и чтения DataFlow
    void ControlThreadParse(bool StartOrStop);
    //узнать состояние потока запущен/нет
    bool GetStatusThreadParse();
    //узнать, заморожен ли поток
    bool GetStatusSuspendParse();
    //открыта ли БД
    bool GetStatusOpenDB();
    //получить ошибки, при вызове очищается структура
    QString GetLastErrorTH();
    QString GetLastError();

    //получить данные из потока записи БД в очередь
    void GetParameters(qint32 &id, QString &time);

public slots:
    void ReceiveError(structDBErrors stDBErr);

//private: //пока что для тестов
    public:

    QString Err;
    ThreadQueue *thq;
    QSqlDatabase *sdb_k;
    QString _lasterr;
    QString currentID;



private:

    bool CreateTables();

    bool wait;
    qint32 cnt;
    QThread* parser_th;




};



#endif // DB_CONTROL_H
