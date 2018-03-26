#include "cthTranslate.h"

cthTranslate::cthTranslate(QVector<QQueue<structDataFlow> *> *DataFlow, QQueue<structDataFlow> *DataFlowToDB, QVector<bool> *vecFlgQueueWr, DBSemaphores *sDBSem)
{
    this->DataFlow=DataFlow;
    this->DataFlowToDB=DataFlowToDB;
    this->vecFlgQueueWr=vecFlgQueueWr;
    this->sDBSem=sDBSem;
    TimeoutToWrite=false;
    Stop=false;
}

void cthTranslate::s_OnTimer()
{
    TimeoutToWrite=true;
    pTimer->stop();
}

void cthTranslate::run()
{
    // Объявляем Таймер
    pTimer = new QTimer(this);
    connect(pTimer, SIGNAL(timeout()), this, SLOT(s_OnTimer(void)), Qt::DirectConnection );
    pTimer->setInterval(Timer_interval);
    pTimer->start();
    QEventLoop loop;

    quint32 i=0;

    //пока нет команды на остановку
    while(!Stop)
    {
        // прокрутили очередь сообщений + таймеры
        loop.processEvents();

        //если время мучения очереди вышло
        if (TimeoutToWrite)
        {
            QThread::msleep(sSleepMS); //поспим
            //если очередь БД пуста
            if (DataFlowToDB->isEmpty())
            {
                //включим таймер
                pTimer->start();
                //сбросим флаг
                TimeoutToWrite=false;
            }
        }
        else//перебираем очереди
            ShiftElements(i);
    }

    //пост действия
    bool dataNull=false, notEmpty=false;

    while(!dataNull)
    {

        for (qint32 j=0;j<vecFlgQueueWr->size()-1;j++)
        {
            if (!DataFlow->operator [](i)->isEmpty())
            {
                notEmpty=true;
                break;
            }
        }

        if (notEmpty)
        {
        notEmpty=false;
        for (i=0;i<vecFlgQueueWr->size();i++)
            ShiftElements(i);
        }
        else dataNull=true;

        QThread::msleep(sSleepMS); //поспим
    }
}

void cthTranslate::ShiftElements(quint32 &i)
{
    // if (!TimeoutToWrite) return;

    //если прошли все очереди - поспим
    if (i>vecFlgQueueWr->size()-1)
    {
        QThread::msleep(sSleepMS); //поспим
        i=0;
        //continue;
        return;
    }

    //если очередь занята или пуста, проверяем следующую очередь
    if (vecFlgQueueWr->operator [](i) || DataFlow->operator [](i)->isEmpty())
    {
        i++;
        //continue;
        return;
    }

    //ставим флаг что пишем в очередь БД
    sDBSem->QueueWr=true;

    while (!DataFlow->operator [](i)->isEmpty())
    {
        //если в очередь хотят записать то переходим к следующей
        if (vecFlgQueueWr->operator [](i))
        {
            i++;
            break;
        }

        //перетаскиваем данные
        DataFlowToDB->enqueue(DataFlow->operator [](i)->dequeue());

    }
    //сбросим флаг
    sDBSem->QueueWr=false;

}

void cthTranslate::StopWork()
{
    Stop=true;
}

























