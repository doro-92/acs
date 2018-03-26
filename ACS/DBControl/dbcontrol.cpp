#include "dbcontrol.h"

DBControl::DBControl(QQueue <structDataFlow> *DataFlowToDB, QQueue<structNewDataFlow> *DataFlowFromDB,  DBSemaphores *sDBSem)
{
    sdb_k = new QSqlDatabase();
    *sdb_k = QSqlDatabase::addDatabase("QSQLITE");

    //регистрируем свой тип данных
    qRegisterMetaType<structDBErrors>();

    //!правильная реализация потоков в QT, единственное память класса
     //ThreadQueue в хипе остается на месте, но пох

    //создадим обьект класса QThread
    parser_th=new QThread();
    //создадим обьект своего класса
    thq=new ThreadQueue(sdb_k,DataFlowToDB, DataFlowFromDB,sDBSem);
    //переместим свой обьект в созданный поток
    thq->moveToThread(parser_th);
    //соединим нашу функцию ThreadQueue::run с сигналом QThread::started
    connect(parser_th, SIGNAL(started()), thq, SLOT(run()));

    //закноннектим свой эррор из потока к классу
    connect(thq, SIGNAL(SendError(structDBErrors)),this,SLOT(ReceiveError(structDBErrors)), Qt::QueuedConnection);
}

DBControl::~DBControl()
{
    if (parser_th->isRunning())
    {
        thq->SuspendThread(true);
        //ожидаем поток
        while(!thq->GetSuspendStatus()) {};
        parser_th->terminate();
    }

    //parser_th->terminate();
    delete [] parser_th;
    delete [] thq;


    sdb_k->close();
    sdb_k->~QSqlDatabase();
    delete [] sdb_k;
}

void DBControl::ClearErrors()
{
    memset(&DBErrors, 0, sizeof(structDBErrors));
}

bool DBControl::CreateDB(QString db_path, bool rewrite)
{
    if (QFileInfo::exists(db_path)&&!rewrite)
    {
        DBErrors.Create=true;
        _lasterr="Файл "+db_path+" существует!";
        return false;
    }

    //перезаписываем если выставлен флаг
    if (QFileInfo::exists(db_path)&&rewrite)
    {
        if (!QFile(db_path).remove())
        {
            DBErrors.Create=true;
            _lasterr="Не возможно удалить файл "+db_path+"!";
            return false;
        }

    }

    // создаем бд
    sdb_k->setDatabaseName(db_path);

    if (!sdb_k->open())
    {
        DBErrors.Create=true;
        DBErrors.Open=true;
        _lasterr = sdb_k->lastError().text();
        return false;
     }
    //создаем таблицы
    if (!CreateTables()) return false;

  return true;
}

bool DBControl::OpenDB(QString db_path)
{
    if (!QFileInfo::exists(db_path))
    {
        DBErrors.Open=true;
        _lasterr="Файл "+db_path+" не существует!";
        return false;
    }

    // создаем бд
    sdb_k->setDatabaseName(db_path);

    if (!sdb_k->open())
    {
        DBErrors.Open=true;
        _lasterr = sdb_k->lastError().text();
        return false;
    }
    return true;
}

void DBControl::CloseDB()
{
    thq->SuspendThread(true);
    while(!thq->GetSuspendStatus()) {};
    sdb_k->close();
}

bool DBControl::CreateTables()
{
    QSqlQuery query(*sdb_k);

    QString str = "CREATE TABLE IF NOT EXISTS 'Session' ("
                  "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "'dt' TEXT NOT NULL,"
                  "'id_data_result' INTEGER);";
                  /*to do*/

    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу сессии: "+sdb_k->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'Config' ("
            "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
            "'id_session' INTEGER,"
            "'num_dev' TEXT,"
            "'param_number' INTEGER," //имя параметра
            "'coefficient' TEXT, " //коэффициент
            "'unit' TEXT," //единица изм
            "'param_name' TEXT );"; //имя параметра


    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу конфигурации: "+sdb_k->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'data' ("
            "'id' INTEGER NOT NULL, "
            "'dt' INTEGER, "
            "'data' TEXT );";


    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу данных: "+sdb_k->lastError().text();
        return false;
    }

    return true;
}


bool DBControl::CreateSession()
{
    QSqlQuery query(*sdb_k);

    if (!query.exec("insert into Session(dt, id_data_result) values(datetime('now','localtime'),0);"))
    {
        DBErrors.WriteToDB=true;
        _lasterr="Невозможно записать в таблицу сессии! Подробности:"
                +sdb_k->lastError().text();
        return false;
    }


    if (!query.exec("SELECT MAX(id) as 'id_m' FROM Session;"))
    {
        DBErrors.ReadFromDB=true;
        _lasterr="Невозможно прочитать таблицу сессии! Подробности:"
                +sdb_k->lastError().text();
        return false;
    }

    QSqlRecord rec     = query.record();
    query.first();
    currentID=query.value(rec.indexOf("id_m")).toString();

    return true;
}

bool DBControl::AddConfig(QString uniq_num_dev, QVector <ConfigStruct> *Config)
{
    //QString num_dev=QString::number(uniq_num_dev);
     QSqlQuery query(*sdb_k);
     QString str;

    //включаем кэширование
    if (!sdb_k->transaction())
    {
        DBErrors.Commit=true;
        _lasterr ="Невозможно включить макротранзакцию: "+sdb_k->lastError().text();
        return false;
    }


    for (int i=0;i<Config->size();i++)
    {
        str="INSERT INTO config(id_session,num_dev,param_number,coefficient,unit,param_name)"
             " VALUES('"+currentID+"','"+
             uniq_num_dev+"', '"+QString::number(Config->operator [](i).param_number)
             +"','"+Config->operator [](i).coefficient+"','"+
             Config->operator [](i).unit+"','"+Config->operator [](i).param_name+"');";

        //qDebug()<<str;
        if (!query.exec(str))
        {
            DBErrors.WriteToDB=true;
            _lasterr ="Невозможно добавить данные в таблицу конфигурации: "+sdb_k->lastError().text();
            return false;
        }

    }

   //заполняем конфигурацию
    if (!sdb_k->commit())
    {
        DBErrors.Commit=true;
         _lasterr ="Ошибка фиксации транзакций: "+sdb_k->lastError().text();
        return false;
    }

    return true;
}

bool DBControl::FlushNewConfig()
{
    return thq->FlushNewConfig(currentID);
}

void DBControl::ControlThreadParse(bool StartOrStop)
{
    //если надо запуститься и поток запущен
    if (StartOrStop)
        if (thq->GetSuspendStatus()) //проверим статус, может заморожен ли
            thq->SuspendThread(false); //разморозим поток

    //если надо запуститься
    if (StartOrStop && !parser_th->isRunning())
        //вызовем событие ThreadQueue::QThread::start()
        parser_th->start();

    //если надо приостановиться и поток запущен
    if (!StartOrStop && !thq->GetSuspendStatus())
        thq->SuspendThread(true); //заморозим его
}

bool DBControl::GetStatusThreadParse()
{
    if (parser_th!=NULL)
        return parser_th->isRunning();
    return false;
}

bool DBControl::GetStatusSuspendParse()
{
    if (thq!=NULL)
        return thq->GetSuspendStatus();
    return false;
}

bool DBControl::GetStatusOpenDB()
{
    return sdb_k->isOpen();
}

QString DBControl::GetLastErrorTH()
{
   ClearErrors();
   return /*thq->Err+*/sdb_k->lastError().text();
}

QString DBControl::GetLastError()
{
    ClearErrors();
    return _lasterr+sdb_k->lastError().text();
}

void DBControl::GetParameters(qint32 &id, QString &time)
{
    thq->GetDBData(id,time);
}

void DBControl::ReceiveError(structDBErrors stDBErr)
{
    DBErrors=stDBErr;
}








