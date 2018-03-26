#include "threadqueue.h"

ThreadQueue::ThreadQueue(QSqlDatabase *sdb_u, QQueue <structDataFlow> *DataFlowToDB, QQueue<structNewDataFlow> *DataFlowFromDB, DBSemaphores *sDBSem)
{
    CurrentSessionConfig = new QVector <structCurrentDataConfig>;
    this->sdb=sdb_u;
    this->DataFlowToDB=DataFlowToDB;
    this->DataFlowFromDB=DataFlowFromDB;
    this->sDBSem=sDBSem;
    wait=false;
    TimerSkip=false;
    cSuspend=false;
    Suspend=true;
    //sem->WrQueueDB=false;
    //run();
}


void ThreadQueue::s_OnTimer(void)
{
    //если нет транзакций, зачем коммитить?
    if (cnt==0) return;

    //пропустим таймер если вычитываем из БД
    if (TimerSkip) return;

    //установим флаг что коммитим
    wait=true;
    cnt=0;
    if (!sdb->commit())
    {
         Err="Ошибка фиксации транзакций: "+sdb->lastError().text();
         wait=false;
         structDBErrors objDBErrs;
         objDBErrs.CacheCommit=true;
         objDBErrs.THError=true;
         emit SendError(objDBErrs);
        Suspend=true;
    }
   //и снова включаем кэширование
   if (!sdb->transaction())
   {
       structDBErrors objDBErrs;
       objDBErrs.CacheCommit=true;
       objDBErrs.THError=true;
       emit SendError(objDBErrs);
         Err="Невозможно включить макротранзакцию: "+sdb->lastError().text();
         wait=false;
         Suspend=true;
   }
   wait=false;
}

bool ThreadQueue::GetDataFromDB(const qint32 &id, const QString &time)
{
    QSqlQuery query2(*sdb);
    QSqlRecord rec;
    QString str;
    structNewDataFlow structOut;

    TimerSkip=true; //таймер пусть пропускает
    //отключим кэширование запросов
    if (!sdb->commit())
    {
        structDBErrors objDBErrs;
        objDBErrs.Commit=true;
        objDBErrs.THError=true;
        emit SendError(objDBErrs);
        Err="Ошибка фиксации транзакций: "+sdb->lastError().text();
        return false;
    }

    //если время задано, то выгрузим от этого времени
    //иначе все что есть
    if (time!="0")
     str="select * from 'data' where id="+QString::number(id)+
            " and dt>"+time+";";
    else
     str="select * from 'data' where id="+QString::number(id)+";";

    if (!query2.exec(str))
    {
        structDBErrors objDBErrs;
        objDBErrs.WriteToDB=true;
        objDBErrs.THError=true;
        emit SendError(objDBErrs);

        Err="Невозможно извлечь данные: "+sdb->lastError().text();
        return false;
    }

    rec = query2.record();

    //установим флаг записи  в очередь на вывод
    sDBSem->DBOutputWR=true;

    while (query2.next()) //переберем все записи
    {
            //сформируем пару и запихнем в очередь
        structOut.id= query2.value(rec.indexOf("id")).toUInt();
        structOut.time=query2.value(rec.indexOf("dt")).toString();
        structOut.data=query2.value(rec.indexOf("data")).toString();

        //добавим в очередь
         DataFlowFromDB->enqueue(structOut);
    }
    sDBSem->DBOutputWR=false; //типа мы все

    //и снова включаем кэширование
    if (!sdb->transaction())
    {
        structDBErrors objDBErrs;
        objDBErrs.CacheCommit=true;
        objDBErrs.THError=true;
        emit SendError(objDBErrs);
          Err="Невозможно включить макротранзакцию: "+sdb->lastError().text();
          return false;
    }
    TimerSkip=false; //включим таймер

    sDBSem->WrQueueDB=false; //сбросим флаг необходимости извлечь данные

    //если данных нет то поспим?
    if (query2.size()==-1)
        QThread::msleep(SleepMS); //поспим

    return true;
}

void ThreadQueue::run()
{
    // Объявляем Таймер
     pTimer = new QTimer(this);
     connect(pTimer, SIGNAL(timeout()), this, SLOT(s_OnTimer(void)), Qt::DirectConnection );
     pTimer->start(Timer_interval);

     cnt=0;
     QSqlQuery query(*sdb);
     QSqlRecord rec;
     QString str;

     //структура для хранения ввода или вывода данных
     structDataFlow structIn;

     QEventLoop loop;
/*
     if (!sdb->transaction())
     {
           //пошлем сообщение основному классу
           structDBErrors objDBErrs;
           objDBErrs.CacheCommit=true;
           objDBErrs.THError=true;
           emit SendError(objDBErrs);
           Err="Невозможно включить макротранзакцию: "+sdb->lastError().text();
           Suspend=true;
           //return;
     }
*/
     while (true)
     {
         // прокрутили очередь сообщений + таймеры
         //exec();
         loop.processEvents();
         //ожидаем запись по таймеру
         if (wait) { QThread::msleep(SleepMS); continue; }

         //если можно поспать
         if (Suspend)
         {
             QThread::msleep(SleepMS);
             continue;
         }

         //если надо завершаться, очередь пуста, очередь не занята, транзакций 0, и негочего извлекать
         if (cSuspend && DataFlowToDB->count()==0 && !sDBSem->QueueWr && cnt==0 && !sDBSem->WrQueueDB)
         {
             TimerSkip=true;
             Suspend=true;
             sdb->commit(); //пофиг - коммитим
             continue;
         }


         //если очередь пустая или занята, что нам париться?
         //можем почитать из БД
         if (DataFlowToDB->count()==0 || sDBSem->QueueWr)
         {
             //если необходимо извлечь данные по запросу
             if (sDBSem->WrQueueDB)
             {
                if (!GetDataFromDB(id,time)) return;
             }
             else //нет запроса
                  QThread::msleep(SleepMS); //поспим

             //и пропустили шаг цикла, мало ли чего произошло пока тут читали или спали
             continue;
         }


         //считаем транзакции
         cnt++;

         sDBSem->QueueRd=true;
         structIn=DataFlowToDB->dequeue();
         sDBSem->QueueRd=false;

         //получаем id
         qint32 currId = GetCurrentIdFromNumParam(QString::number(structIn.numDevice),structIn.numParam);

//         if (currId==-1)
//         {
//             qDebug()<<"Error id="<<QString::number(structIn.numDevice)<<","<<structIn.numParam;
//         }

        // qDebug()<<QString::number(currId)<<","<<structIn.time<<","<<structIn.data;

         str = " INSERT INTO 'data' VALUES('"+QString::number(currId)
                 +"','"+structIn.time+"','"+structIn.data+"');";

         bool t=query.exec(str);
         if (!t)
         {
             structDBErrors objDBErrs;
             objDBErrs.WriteToDB=true;
             objDBErrs.THError=true;
             emit SendError(objDBErrs);
             Err="Невозможно добавить данные: "+sdb->lastError().text();
             Suspend=true;
               //return;
         }

     };
}


void ThreadQueue::GetDBData(qint32 &id, QString &time)
{
    //если запрос есть то сбросим вызов
    if (sDBSem->WrQueueDB) return;
    this->id=id;
    this->time=time;
    sDBSem->WrQueueDB=true;
}

void ThreadQueue::SuspendThread(bool flgSuspend)
{
    cSuspend=flgSuspend;
    if (!cSuspend)
    {
        Suspend=false;
        TimerSkip=false;
        sdb->transaction();
    }
}

bool ThreadQueue::GetSuspendStatus()
{
    return Suspend;
}

bool ThreadQueue::FlushNewConfig(QString &currentID)
{
    QSqlQuery query(*sdb);

    if (!query.exec("SELECT * FROM Config where id_session="+currentID+";"))
    {
        structDBErrors objDBErrs;
        objDBErrs.ReadFromDB=true;
        emit SendError(objDBErrs);
        Err="Невозможно прочитать таблицу конфигурации! Подробности:"
                +sdb->lastError().text();
        return false;
    }

    QSqlRecord rec     = query.record();
    CurrentSessionConfig->clear();

    while (query.next()) //переберем все записи
    {
        structCurrentDataConfig structDC;
            //сформируем пару и запихнем в структуру
        structDC.ID = query.value(rec.indexOf("id")).toUInt();
        structDC.numDevice=query.value(rec.indexOf("num_dev")).toString();
        structDC.param_number=query.value(rec.indexOf("param_number")).toInt();
        structDC.param_name=query.value(rec.indexOf("param_name")).toInt();
        structDC.unit=query.value(rec.indexOf("unit")).toString();
        structDC.coefficient=query.value(rec.indexOf("coefficient")).toString();

        //добавим в вектор
         CurrentSessionConfig->push_back(structDC);
    }

    return true;
}

qint32 ThreadQueue::GetCurrentIdFromNumParam(const QString &numDevice, const qint16 &numParam)
{
   for (qint32 i=0;i<CurrentSessionConfig->size();i++)
       if (CurrentSessionConfig->operator [](i).numDevice==numDevice &&
               CurrentSessionConfig->operator [](i).param_number==numParam)
           return CurrentSessionConfig->operator [](i).ID;
   return -1;
}

bool ThreadQueue::GetCurrentNumParamFromId(const quint32 id, QString &numDevice, quint32 &numParam)
{
   for (qint32 i=0;i<CurrentSessionConfig->size();i++)
       if (CurrentSessionConfig->operator [](i).ID==id)
       {
           numDevice=CurrentSessionConfig->operator [](i).numDevice;
           numParam=CurrentSessionConfig->operator [](i).param_number;
           return true;
       }
   return false;
}
