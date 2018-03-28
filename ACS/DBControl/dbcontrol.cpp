#include "dbcontrol.h"

DBControl::DBControl(QQueue <structDataFlow> *DataFlowToDB, QQueue<structNewDataFlow> *DataFlowFromDB,  DBSemaphores *sDBSem)
{
    sdb = new QSqlDatabase();
    *sdb = QSqlDatabase::addDatabase("QSQLITE");

    //регистрируем свой тип данных
    qRegisterMetaType<structDBErrors>();

    //!правильная реализация потоков в QT, единственное память класса
     //ThreadQueue в хипе остается на месте, но пох

    //создадим обьект класса QThread
    parser_th=new QThread();
    //создадим обьект своего класса
    thq=new ThreadQueue(sdb,DataFlowToDB, DataFlowFromDB,sDBSem);
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


    sdb->close();
    sdb->~QSqlDatabase();
    delete [] sdb;
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
    sdb->setDatabaseName(db_path);

    if (!sdb->open())
    {
        DBErrors.Create=true;
        DBErrors.Open=true;
        _lasterr = sdb->lastError().text();
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
    sdb->setDatabaseName(db_path);

    if (!sdb->open())
    {
        DBErrors.Open=true;
        _lasterr = sdb->lastError().text();
        return false;
    }
    return true;
}

void DBControl::CloseDB()
{
    thq->SuspendThread(true);
    while(!thq->GetSuspendStatus()) {};
    sdb->close();
}

bool DBControl::CreateTables()
{
    QSqlQuery query(*sdb);

    QString str = "CREATE TABLE IF NOT EXISTS 'Session' ("
                  "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "'dt' TEXT NOT NULL,"
                  "'serial' TEXT NOT NULL,"
                  "'testOk' INTEGER);";
                  /*to do*/

    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу сессии: "+sdb->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'Rule' ("
                  "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "'id_session' INTEGER NOT NULL,"
                  "'code' TEXT);";

    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу правил: "+sdb->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'Mode' ("
                  "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "'id_session' INTEGER NOT NULL,"
                  "'num_mode' INTEGER NOT NULL);";

    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу режимов: "+sdb->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'Config' ("
            "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
            "'id_mode' INTEGER,"
            "'num_dev' TEXT,"
            "'param_number' INTEGER," //номер параметра
            "'coefficient' TEXT, " //коэффициент
            "'unit' TEXT," //единица изм
            "'param_name' TEXT );"; //имя параметра


    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу конфигурации: "+sdb->lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS 'Data' ("
            "'id_config' INTEGER NOT NULL, "
            "'dt' INTEGER, "
            "'data' TEXT );";


    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        DBErrors.CreateTable=true;
        _lasterr = "Не удается создать таблицу данных: "+sdb->lastError().text();
        return false;
    }

    return true;
}


bool DBControl::CreateSession()
{
    QSqlQuery query(*sdb);

    if (!query.exec("insert into Session(dt,serial, testOk) "
                    "values(datetime('now','localtime'),"+SerialNumber+",0);"))
    {
        DBErrors.WriteToDB=true;
        _lasterr="Невозможно записать в таблицу сессии! Подробности:"
                +sdb->lastError().text();
        return false;
    }


    if (!query.exec("SELECT MAX(id) as 'id_m' FROM Session;"))
    {
        DBErrors.ReadFromDB=true;
        _lasterr="Невозможно прочитать таблицу сессии! Подробности:"
                +sdb->lastError().text();
        return false;
    }

    QSqlRecord rec     = query.record();
    query.first();

    if (query.value(rec.indexOf("id_m")).toInt()==0)
    {
        DBErrors.ReadFromDB =true;
        _lasterr="Не найден номер сессии в БД! Подробности:"
                +sdb->lastError().text();
        return false;
    }

    currentID=query.value(rec.indexOf("id_m")).toString();

    return true;
}


bool DBControl::AddMode(QString numMode)
{
    QSqlQuery query(*sdb);

    QString str="INSERT INTO Mode(id_session,num_mode) "
            "VALUES("+currentID+","+numMode+")";

    if (!query.exec(str))
    {
        DBErrors.WriteToDB=true;
        _lasterr ="Невозможно добавить данные в таблицу режима: "+sdb->lastError().text();
        return false;
    }

    if (!query.exec("SELECT MAX(id) as 'id_m' FROM Mode;"))
    {
        DBErrors.ReadFromDB=true;
        _lasterr="Невозможно прочитать таблицу режима! Подробности:"
                +sdb->lastError().text();
        return false;
    }

    QSqlRecord rec     = query.record();
    query.first();

    if (query.value(rec.indexOf("id_m")).toInt()==0)
    {
        DBErrors.ReadFromDB =true;
        _lasterr="Не найден номер режима в БД! Подробности:"
                +sdb->lastError().text();
        return false;
    }

    currentMode=query.value(rec.indexOf("id_m")).toString();


    return true;
}

bool DBControl::AddConfig(QString uniq_num_dev, QVector <ConfigStruct> *Config)
{
    //QString num_dev=QString::number(uniq_num_dev);
     QSqlQuery query(*sdb);
     QString str;

    //включаем кэширование
    if (!sdb->transaction())
    {
        DBErrors.Commit=true;
        _lasterr ="Невозможно включить макротранзакцию: "+sdb->lastError().text();
        return false;
    }

/*
            "'id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
            "'id_mode' INTEGER,"
            "'num_dev' TEXT,"
            "'param_number' INTEGER," //номер параметра
            "'coefficient' TEXT, " //коэффициент
            "'unit' TEXT," //единица изм
            "'param_name' TEXT );"; //имя параметра
*/

    for (int i=0;i<Config->size();i++)
    {
        str="INSERT INTO Config(id_mode,num_dev,param_number,coefficient,unit,param_name)"
             " VALUES('"+currentMode+"','"+
             uniq_num_dev+"', '"+QString::number(Config->operator [](i).param_number)
             +"','"+Config->operator [](i).coefficient+"','"+
             Config->operator [](i).unit+"','"+Config->operator [](i).param_name+"');";

        //qDebug()<<str;
        if (!query.exec(str))
        {
            DBErrors.WriteToDB=true;
            _lasterr ="Невозможно добавить данные в таблицу конфигурации: "+sdb->lastError().text();
            return false;
        }

    }

   //заполняем конфигурацию
    if (!sdb->commit())
    {
        DBErrors.Commit=true;
         _lasterr ="Ошибка фиксации транзакций: "+sdb->lastError().text();
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
    return sdb->isOpen();
}

QString DBControl::GetLastErrorTH()
{
   ClearErrors();
   return /*thq->Err+*/sdb->lastError().text();
}

QString DBControl::GetLastError()
{
    ClearErrors();
    return _lasterr+sdb->lastError().text();
}

void DBControl::GetParameters(qint32 &id, QString &time)
{
    thq->GetDBData(id,time);
}

void DBControl::ReceiveError(structDBErrors stDBErr)
{
    DBErrors=stDBErr;
}








