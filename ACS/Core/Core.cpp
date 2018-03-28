#include "Core.h"

Core::Core(const quint32 &numDFqueue, QWidget *ProgWgt, QWidget *PlotWgt, QPlainTextEdit *plainTextEdit)
{
    //init core systems
    this->numDFqueue=numDFqueue;
    for (quint32 i=0;i<numDFqueue;i++)
        vecDataFlow.push_back(new QQueue <structDataFlow>);

    //DataFlow = new QQueue <structDataFlow>[numDFqueue];
    DataFlowToDB = new QQueue <structDataFlow>;
    DataFlowFromDB = new QQueue <structNewDataFlow>;
    for (quint32 i=0;i<numDFqueue;i++)
        vecFlgQueueWr.push_back(false);

    sDBSem = new DBSemaphores();

    objTH = new QThread();
    objThTranslate = new cthTranslate(&vecDataFlow,DataFlowToDB,&vecFlgQueueWr,sDBSem);
    objThTranslate->moveToThread(objTH);
    //соединим нашу функцию ThTranslate::run с сигналом QThread::started
    connect(objTH, SIGNAL(started()), objThTranslate, SLOT(run()));
    objTH->start();

    ///////////////////////////////////////////////////////////////////////

    //инициализация девайса/ов
    objPLCF5A = new device_PLC_F5A(ProgWgt,vecDataFlow[0],&vecFlgQueueWr[0]);
    //...

    //инициализация БД
    objDBC = new DBControl(DataFlowToDB,DataFlowFromDB,sDBSem);
    //..

    //инициализация правил
    objRule = new RuleControl(plainTextEdit);

    //инициализация графика
    objQPlot = new QPlot(PlotWgt);
}

bool Core::OpenCreateDBFromSerialNumber(QString serialNumber)
{
    if (!QDir(qApp->applicationDirPath()+"/data_db/").exists())
        if (!QDir().mkdir(qApp->applicationDirPath()+"/data_db/"))
        {
            QMessageBox::critical(NULL,"Ошибка","Не возможно создать папку: "+qApp->applicationDirPath()+"/data_db/");
            return false;
        }

    if (!QDir(qApp->applicationDirPath()+"/image_db/").exists())
        if (!QDir().mkdir(qApp->applicationDirPath()+"/image_db/"))
        {
            QMessageBox::critical(NULL,"Ошибка","Не возможно создать папку: "+qApp->applicationDirPath()+"/image_db/");
            return false;
        }

    QString db_path=qApp->applicationDirPath()+"/data_db/"+serialNumber+"_data.sqlite3";

    //QString db_filename = QFileDialog::getOpenFileName(this, tr("Отрыть файл.."), "." , tr("*"));
    //есть ли бд?
    if (QFileInfo::exists(db_path))
    {
        //бд существует, значит откроем
        if (!objDBC->OpenDB(db_path))
        {
             QMessageBox::critical(NULL,"Ошибка",objDBC->GetLastError()+"!");
             return false;
        }
        return true;
     }

    //если нет - создаем
    if (!objDBC->CreateDB(db_path,false))
    {
        QMessageBox::critical(NULL,"Ошибка",objDBC->GetLastError()+"!");
        return false;
    }

    return true;
}

bool Core::GetStatusOpenDB()
{
    return objDBC->GetStatusOpenDB();
}

void Core::CloseDB()
{
    objDBC->CloseDB();
}

bool Core::GetSuspendStatusDB()
{
    return objDBC->GetStatusSuspendParse();
}

bool Core::StartTest()
{
    if (!objDBC->CreateSession())
    {
        QMessageBox::critical(NULL,"Ошибка","Не возможно создать сессию! Подробности: "+objDBC->GetLastError());
        return false;
    }

    QVector <ConfigStruct> tmp=objPLCF5A->getConfigVector();
    if (!objDBC->AddConfig("1",&tmp))
    {
        QMessageBox::critical(NULL,"Ошибка","Не возможно записать конфигурацию в БД! Подробности: "+objDBC->GetLastError());
        return false;
    }

    if (!objDBC->FlushNewConfig())
    {
        QMessageBox::critical(NULL,"Ошибка","Не возможно создать вектор конфигураций! Подробности: "+objDBC->GetLastError());
        return false;
    }

    objDBC->ControlThreadParse(true);
    QThread::msleep(5); //без него конечно работает, но на всякий случай

    if (!objDBC->GetStatusThreadParse() || objDBC->GetStatusSuspendParse())
    {
        QMessageBox::critical(NULL,"Ошибка","Поток не может быть запущен! Подробности: "+objDBC->GetLastError());
        return false;
    }

    return true;

}

void Core::TestSetDataFlow(QVector<structDataFlow> *in_vecDataFlow)
{
    for (quint32 i=0;i<numDFqueue;i++)
    {
        vecFlgQueueWr[i]=true;
        for (qint32 j=0;j<in_vecDataFlow->size();j++)
        {
            vecDataFlow[i]->enqueue(in_vecDataFlow->operator [](j));
//            qDebug()<<j;
        }
        vecFlgQueueWr[i]=false;

    }
    return;
}

void Core::TestSetFlgDataFlow(quint32 i, bool flg)
{
    vecFlgQueueWr[i]=flg;
}

void Core::TestControlDBThread()
{
    objDBC->ControlThreadParse(objDBC->GetStatusSuspendParse());
}

QString Core::TestGetLastErrorDB()
{
    return objDBC->GetLastErrorTH();
}

bool Core::GetFlgDataFlow(qint32 i)
{
    return vecFlgQueueWr[i];
}

DBSemaphores Core::GetFlgDB()
{
    return *sDBSem;
}

quint32 Core::GetCountDFQueue()
{
    return numDFqueue;
}

bool Core::GetIsEmptyDFdb()
{
    return DataFlowToDB->isEmpty();
}

bool Core::GetStatusThDB()
{
    return objDBC->GetStatusSuspendParse();
}
