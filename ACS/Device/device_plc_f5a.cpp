#include "Device/device_plc_f5a.h"
#include "EXT/ext.h"

#define time_connect 2000      //период работы таймера подключения к серверу
#define time_send 2000         //период работы таймера отправки данных






device_PLC_F5A::device_PLC_F5A (QWidget* pwgt,
                                QQueue<structDataFlow> *p_queue_to_db_in,
                                bool *flgWriteToQu
                                /*, bool *glob_flag*/) : QWidget(pwgt), next_block_size(0)
{
    this->setStyleSheet("background-color: #f1f1f1;"                    //настройка стиля отображениявиджетов
                        "QTextEdit {background-color: #ffffff;}");


    fillConfigVector();                     //заполнение вектора конфигурации параметров


    /*--------?-------------*/
    p_queue_to_db=p_queue_to_db_in;         //присвоение значений указателей (поступивших параметрами в конструктор) указателям, используемым в методах этого класса
    this->flgWriteToQu=flgWriteToQu;
    /*--------?-------------*/


    p_tcp_socket = new QTcpSocket(this);                                            //Создание ТСР сокета
    connect(p_tcp_socket, SIGNAL(connected()), SLOT(slot_connected()));             //Подключение слота "Сокет подключен" к соотвествующему сигналу сокета
    connect(p_tcp_socket, SIGNAL(disconnected()), SLOT(slot_disconnected()));       //Подключение слота "Сокет отключен" к соотвествующему сигналу сокета
    connect(p_tcp_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(slot_error(QAbstractSocket::SocketError)) );                       //Подключение слота "Сокет отключен" к соотвествующему сигналу сокета
    connect(p_tcp_socket, SIGNAL(readyRead()), SLOT(slot_ready_read()));          //Подключение слота "Сокет имеет полученную информацию" к соотвествующему сигналу сокета



    p_timer_connect = new QTimer(this);                                                 //Создание таймера подключения к серверу
    connect(p_timer_connect, SIGNAL(timeout()), this, SLOT(timer_connect_tick()) );     //подключение слота попытки подключения к сигналу таймера
    p_timer_connect->start(time_connect);                                               //запуск таймера попытки подключения с периодом 2 сек

    p_timer_send = new QTimer(this);                                                    //Создание таймера отправки дейтаграмм
    connect(p_timer_send, SIGNAL(timeout()), this, SLOT(slot_send_to_server()) );     //Подключение слота отправки дейтаграмм к таймеру отправки дейтаграмм
    p_timer_send->start(time_send);                                                     //запуск таймера отправки с периодом 1 сек



    QVBoxLayout* p_vbx_layout_1 = new QVBoxLayout(this);                         //создание менеджера компоновки 1-го уровня

    QLabel* p_label = new QLabel("Клиент device_PLC_F5A");                       //создание и расположение надписи заголовка виджета device_PLC_F5A
    p_vbx_layout_1->addWidget(p_label);


    QTabWidget* p_tab_widget = new QTabWidget();                                //создание и расположение виджета вкладок

        QWidget *p_tab_widget__tab_1 = new QWidget(p_tab_widget);                           //создание 1-й вкладки
        p_tab_widget->addTab(p_tab_widget__tab_1, "Диагностический буфер");
        QVBoxLayout* p_vbx_layout_2 = new QVBoxLayout(p_tab_widget__tab_1);                //создание менеджера компоновки 2-го уровня

        QWidget *p_tab_widget__tab_2 = new QWidget(p_tab_widget);                           //создание 2-й вкладки
        p_tab_widget->addTab(p_tab_widget__tab_2, "Настройки");
        QVBoxLayout* p_vbx_layout_3 = new QVBoxLayout(p_tab_widget__tab_2);                //создание менеджера компоновки 2-го уровня

        QWidget *p_tab_widget__tab_3 = new QWidget(p_tab_widget);                           //создание 3-й вкладки
        p_tab_widget->addTab(p_tab_widget__tab_3, "Параметры");
        QVBoxLayout* p_vbx_layout_4 = new QVBoxLayout(p_tab_widget__tab_3);                //создание менеджера компоновки 2-го уровня

    p_vbx_layout_1->addWidget(p_tab_widget);

                                                                    //интерфейс 1-й вкладки
    p_txt_info_diag_buffer = new QTextEdit;                         //создание, настройки и расположение информационного окна (диагностический буфер)
    p_txt_info_diag_buffer->setReadOnly(true);
    p_vbx_layout_2->addWidget(p_txt_info_diag_buffer);

                                                                    //интерфейс 2-й вкладки
    p_line_info_addr = new QLineEdit;                               //создание, настройки и расположение поля ввода IP адреса
    //del
    p_line_info_addr->setText("192.168.0.100:8013");                //временный код - установка адреса устройства на время разработки данного софта "127.0.0.1:8010" "192.168.0.10:8010"
    p_vbx_layout_3->setAlignment(Qt::AlignTop);
    p_vbx_layout_3->addWidget(p_line_info_addr);
    connect(p_line_info_addr, SIGNAL(textChanged(const QString)),
            SLOT(addr_verify(const QString)) );

    //del
    p_chk_box = new QCheckBox;                                      //создание, настройки и расположение чекбокса корректности IPадреса
    p_vbx_layout_3->addWidget(p_chk_box);

                                                                                        //интерфейс 3-й вкладки

    lst_wdg = new QListWidget;                                                          //создание списка элементов (список параметров устройства)
    foreach(struct ConfigStruct structConf, config_vector)                          //для каждого элемента вектора настроек параметров создается....
    {
        QListWidgetItem* item = new QListWidgetItem(structConf.param_name, lst_wdg);        //...элемент списка...
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);                        //...с утановкой флажка (set checkable flag)...
        item->setCheckState(Qt::Unchecked);                                             //...и инициализацией состояния этого флажка ( AND initialize check state)

    }
    // p_txt_info_diag_buffer->append(  lwg->item(0)->text() );
    //  p_txt_info_diag_buffer->append(  QString::number(lst_wdg->item(0)->checkState() )  );
    p_vbx_layout_4->addWidget(lst_wdg);



        //del
        //QLabel* lll=new QLabel();
        //lll->setText( QString::number( lwg->count() ));
        //p_vbx_layout_4->addWidget(lll);





    //пока закоменнтировал, но было без коммента
    //setLayout(p_vbx_layout_1);                                        //установки  менеджеров компоновки
    //p_tab_widget__tab_1->setLayout(p_vbx_layout_2);

    this->setGeometry(QRect(10,30,400,600));                        //установка размеров виджета device_PLC_F5A

    p_txt_info_diag_buffer->append( diag_string("ТСР сокет создан"));


    //del------------------------------------------------------------------------------------------------

/*
    QVector<ModeStruct> vec;

    ModeStruct mode_1;
    mode_1.number=1;
    mode_1.name="speed";

    ModeStruct mode_2;
    mode_2.number=2;
    mode_2.name="pos";

    vec<<mode_1<<mode_2;


    QVectorIterator<ModeStruct> it (vec);
    while(it.hasNext () )
    {

        p_txt_info_diag_buffer->append( diag_string(it.next().name));


    }
*/

    //del------------------------------------------------------------------------------------------------

}






QString device_PLC_F5A::diag_string (QString str)             //создание строки диагностического сообщения
{
    str= QTime::currentTime().toString("hh:mm:ss.zzz") +
         "   " +
         str +
         "\n";
    return str;
    //return QString("");
}






void device_PLC_F5A::addr_verify(QString text)                                              //проверка правильности введенного IP-адреса
{
    //QRegExp addr_reg("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\:[0-9]{1,5}");       //УБРАНО,пока это действие выполняется при попытке подключения

}







void device_PLC_F5A::timer_connect_tick()               //попытка подключения по ТСР по сигналу таймера при отсуствии подключения
{
   QRegExp addr_reg("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\:[0-9]{1,5}");          //регулярное выражение для определения, корректно ли пользователь ввел IP-адрес устройства
   p_chk_box->setChecked( addr_reg.exactMatch( p_line_info_addr->text() ) );                    //активация чекбокса, если пользователь ввел все верно

   if ( (p_tcp_socket->state() == 0) &                                                          //если сокет не подключен
        (addr_reg.exactMatch(p_line_info_addr->text()))  )                                      //если  указанный IPадрес устройства корректно написан
   {

       QStringList addr_port=p_line_info_addr->text().split(":", QString::SkipEmptyParts);      //разделить строку с адресом и портом на две подстроки (отдельно адрес и отдельно порт)
       strHost=addr_port[0];
       nPort=addr_port[1].toInt();

       p_tcp_socket->connectToHost(strHost, nPort);                                              //попытаться подключиться к серверу
   }

}





void device_PLC_F5A::slot_connected()                                   //слот, выполняемый при установке подключения к ПЛК
{
    p_txt_info_diag_buffer->append( diag_string("ПЛК подключен") );
}






void device_PLC_F5A::slot_disconnected()                                 //слот, выполняемый при отключении от ПЛК
{
    p_txt_info_diag_buffer->append(diag_string("ПЛК отключен") );

}




void device_PLC_F5A::slot_error(QAbstractSocket::SocketError error_code)     //слот, выполняемый при возникновении ошибки подключения ПЛК
{
    p_txt_info_diag_buffer->append(
                           diag_string ( "Ошибка подключения к ПЛК: " +
                           QString::number(error_code)  +
                           " ("                         +
                           p_tcp_socket->errorString()  +
                           ")." )
                       );
}







void device_PLC_F5A::slot_ready_read()
{
    //del
    //sem->QueueWr = true;
    *flgWriteToQu=true;


    QDataStream in(p_tcp_socket);
    in.setVersion(QDataStream::Qt_5_2);
    in.setByteOrder(QDataStream::LittleEndian);


    for (;;)
    {
        if (next_block_size==0)
        {
            if (p_tcp_socket->bytesAvailable() < sizeof(quint32))     //если принято менее четырех байтов (не принята общая длина сообщения)
            {
                break;
            }
            in >> next_block_size;                                  //чтение длины сообщения
        }

        if (p_tcp_socket->bytesAvailable() != next_block_size*(sizeof(quint32)) )         //если принятоекол-во байт меньше общей длины сообщения
        {
            break;
        }

        //общая длина датаграммы
        p_txt_info_diag_buffer->append(  diag_string("Длина датаграммы " + QString::number(next_block_size) +" элементов")  );

        //0я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 0-й части " + QString::number(next_block_size) +" элементов.")  );

        qint32 in_byte_1, in_byte_2, in_byte_3, in_byte;
        for (qint32 i=1; i<=next_block_size/3; i++)
        {
            in>>in_byte_1;    //номер
            in>>in_byte_2;    //значение
            in>>in_byte_3;    //время




//            QPair <qint32, QPair <QString, qint32> > main_data_to_db;     //v1
//            QPair <QString, qint32> second_data_to_db;

//            second_data_to_db.first= QString::number( in_byte_3);
//            second_data_to_db.second=in_byte_2;
//            main_data_to_db.first=in_byte_1;
//            main_data_to_db.second=second_data_to_db;


            structDataFlow sdf;
                sdf.numDevice=1;
                sdf.numParam=in_byte_1;
                sdf.data=QString::number(in_byte_2);
                sdf.time= QString::number( in_byte_3);
            p_queue_to_db->enqueue( sdf );


              //p_txt_info_diag_buffer->append ( QString::number (in_byte_1) + " " + QString::number (in_byte_2) + " " + QString::number (in_byte_3) );
              p_txt_info_diag_buffer->append(  QString::number(sdf.numParam) +
                                               " " + sdf.time + " " +
                                              sdf.data   );     //вывод пар, напрявляемых в очередь БД

              //p_txt_info_diag_buffer->append(QString::number(in_byte) +"  " );                       //вывод сырых пришедших данных

              //p_queue_to_db->enqueue(main_data_to_db);  //ver1                                                 //запись данных в очередь
        }

        //1я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 1-й части " + QString::number(next_block_size) +" элементов")  );

        //2я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 2-й части " + QString::number(next_block_size) +" элементов")  );

        //3я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 3-й части " + QString::number(next_block_size) +" элементов")  );

        //4я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 4-й части " + QString::number(next_block_size) +" элементов")  );

        //5я часть датаграммы
        in >> next_block_size;
        p_txt_info_diag_buffer->append(  diag_string("Длина 5-й части " + QString::number(next_block_size) +" элементов")  );


        next_block_size = 0;        //завершение чтения датаграммы

    } //for

     *flgWriteToQu=false;
    //sem->QueueWr = false;
}






void device_PLC_F5A::slot_send_to_server()                  //отправка данных устройтву по ТСР
{
     if (p_tcp_socket->state()==3)
     {
        QByteArray arrBlock;
        QDataStream out(&arrBlock, QIODevice::WriteOnly);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_0;
        QDataStream out_0(&arrBlock_0, QIODevice::ReadWrite);
        out_0.setByteOrder(QDataStream::LittleEndian);
        out_0.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_1;
        QDataStream out_1(&arrBlock_1, QIODevice::ReadWrite);
        out_1.setByteOrder(QDataStream::LittleEndian);
        out_1.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_2;
        QDataStream out_2(&arrBlock_2, QIODevice::ReadWrite);
        out_2.setByteOrder(QDataStream::LittleEndian);
        out_2.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_3;
        QDataStream out_3(&arrBlock_3, QIODevice::ReadWrite);
        out_3.setByteOrder(QDataStream::LittleEndian);
        out_3.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_4;
        QDataStream out_4(&arrBlock_4, QIODevice::ReadWrite);
        out_4.setByteOrder(QDataStream::LittleEndian);
        out_4.setVersion(QDataStream::Qt_5_2);

        QByteArray arrBlock_5;
        QDataStream out_5(&arrBlock_5, QIODevice::ReadWrite);
        out_5.setByteOrder(QDataStream::LittleEndian);
        out_5.setVersion(QDataStream::Qt_5_2);


        //out << qint32(3);
        //out_0 << qint32(2) << qint32(2) << qint32(1);



        //out << arrBlock_0;
        //out.device()->seek(0);                                //окончательная запись общей длины датаграммы
        //out << qint32( arrBlock.size() - sizeof(qint32) );

        out << qint32(0);           //предварительная запись длины датаграммы


        int chkd_count=0;
        out_0 << qint32(0);
        for (int i = 0; i < lst_wdg->count(); i++)
        {
            if ( lst_wdg->item(i)->checkState() == Qt::Checked)
            {
                chkd_count++;
                out_0 << qint32(i+1);
            }
        }
        out_0.device()->seek(0);                                     //окончательная запись  длины
        //out_0 << qint32( arrBlock_0.size() - sizeof(qint32) );
        out_0 << qint32( arrBlock_0.size()/4 - sizeof(qint32)/4 );



//           int param_count = lst_wdg->count();

//           for (int i = 0; i < lst_wdg->count(); i++)
//            {
//                if ( lst_wdg->item(i)->checkState() == Qt::Unchecked)
//                {
//                    chkd_count++;
//                }
//            }
        //    p_txt_info_diag_buffer->append(  QString::number(chkd_count) );



        //        out.device()->seek(0);                                //окончательная запись общей длины датаграммы
        //        out << qint32( arrBlock.size() - sizeof(qint32) );


        out_1 << qint32(0);     //временные заглушки для ввода содержимого пустых частей датаграммы
        out_2 << qint32(0);
        out_3 << qint32(0);
        out_4 << qint32(0);
        out_5 << qint32(0);


        out.device()->seek(0);                                //окончательная запись общей длины датаграммы
        out << qint32( (arrBlock + arrBlock_0 + arrBlock_1 + arrBlock_2 + arrBlock_3 + arrBlock_4 + arrBlock_5).size()/4 - sizeof(qint32)/4 );




        p_tcp_socket->write(arrBlock + arrBlock_0 + arrBlock_1 + arrBlock_2 + arrBlock_3 + arrBlock_4 + arrBlock_5);
        p_txt_info_diag_buffer->append( diag_string("Отправка"));
    }
}





void device_PLC_F5A::timer_send_tick()    //действие при срабатывании таймера отправки дейтаграммы
{
    p_txt_info_diag_buffer->append("aa");
}







QVector <ConfigStruct> device_PLC_F5A::getConfigVector ()
{

    return (config_vector);
}







void  device_PLC_F5A::fillConfigVector()
{
    /*
    qint16 param_number;
    QString coefficient;
    QString unit;
    QString param_name;
    qint32 param_adress;
    */

    ConfigStruct param;

    param.param_number=1;
    param.coefficient="1";
    param.unit="";
    param.param_name="ru.00 - Статус преобразователя";
    param.param_adress=0x2200;
    config_vector.push_back(param);

    param.param_number=2;
    param.coefficient="1"; "0.125";
    param.unit="об/мин";
    param.param_name="sy.53 - Фактическая скорость"; //"ru.07 - Фактическая скорость";
    param.param_adress=0x2207;
    config_vector.push_back(param);

    param.param_number=3;
    param.coefficient="0.01";
    param.unit="Нм";
    param.param_name="ru.12 - Фактический момент";
    param.param_adress=0x220C;
    config_vector.push_back(param);

    param.param_number=4;
    param.coefficient="1";
    param.unit="Инк";
    param.param_name="ru.54 - Фактическая позиция";
    param.param_adress=0x2236;
    config_vector.push_back(param);
}









//------------------------------------------старый код--------------------------





//p_txt_info_diag_buffer->append(  QString::number(lst_wdg->item(0)->checkState() )  );
//p_txt_info_diag_buffer->append(  QString::number(lst_wdg->count() )  );
//    int chkd_count = 0;
//   for (int i = 0; i < lst_wdg->count(); i++)
//    {
//        if ( lst_wdg->item(i)->checkState() == Qt::Unchecked)
//        {
//            chkd_count++;
//        }
//    }
//    p_txt_info_diag_buffer->append(  QString::number(chkd_count) );










//QLabel* lll=new QLabel();
//lll->setText( QString::number( lwg->count() ));
//p_vbx_layout_4->addWidget(lll);









//     QList <QListWidgetItem*> testlist = lwg->findItems("мом", Qt::MatchContains);
//     foreach (QListWidgetItem* pitem, testlist)
//     {
//         p_txt_info_diag_buffer->append(pitem->text());
//     }


    //lwg->addItem( lwg->item(1) );
//    QListWidgetItem* item22 = new QListWidgetItem("dob");                              //...элемент списка...
//    item22->setFlags(item22->flags() | Qt::ItemIsUserCheckable);                        //...с утановкой флажка (set checkable flag)...
//    item22->setCheckState(Qt::Unchecked);

//    QListWidgetItem* item22 = lwg->it
//    lwg->addItem(item22);

//    QListWidgetItem* item22 = new QListWidgetItem("dob");

//    lwg->insertItem(0, item22);
//    lwg->insertItem(1, item22);
    //QListWidgetItem* item23 = lwg->item(0);









/*
//p_txt_input = new QLineEdit;                                //создание, настройки и расположение строки ввода информации
//connect(m_ptxtlnput, SIGNAL(returnPressed()),
//this, SLOT(slotSendToServer()));
//p_vbx_layout->addWidget(p_txt_input);

QPushButton* pCmdButton = new QPushButton("Отправить");     //создание, настройки и расположение кнопки "Отправить"
connect(pCmdButton, SIGNAL(clicked()),
SLOT(slot_send_to_server()));
p_vbx_layout->addWidget(pCmdButton);

p_chk_box = new QCheckBox;                                  //создание, настройки и расположение чекбокса
connect(p_chk_box,SIGNAL(stateChanged(int)),
this, SLOT(timerStartStop()));
p_vbx_layout->addWidget(p_chk_box);

//Расположение ннтерфейса, вер.2
//QVBoxLayout* pvbxLayout_1 = new QVBoxLayout;
//p_vbx_layout->addLayout(pvbxLayout_1);
//pvbxLayout_1->addWidget(new QLabel("C6+F5A"));

//p_vbx_layout->setGeometry(QRect(10,10,200,200));
*/










/*
        QQueue < QQueue<QString> > que;
        QQueue <QString> subque;

                QQueue <QString,QString> subque1;

        QQueue <QString> que(5);
        que.enqueue("Era");
        que.enqueue("Corvus Corax");
        que.enqueue("Gathering");

        subque.enqueue("a1");
        subque.enqueue("a2");
         que.enqueue(subque);
         subque.clear();

         subque.enqueue("b1");
         subque.enqueue("b2");
          que.enqueue(subque);
          subque.clear();

        subque=que.dequeue();
        p_txt_info_diag_buffer->append(subque.dequeue());
        p_txt_info_diag_buffer->append(subque.dequeue());

        while (!que.empty())
        {
            //qDebug() « "Element:" «que.dequeue();
            p_txt_info_diag_buffer->append(que.dequeue());
        }
*/










//void device_PLC_F5A::timerStartStop()
//{
//    if (p_chk_box->isChecked())
//    {
//        //p_timer_send->start(1000);
//    }
//    else
//    {
//        p_timer_send->stop();
//    }
//}







// СЛОТ readyRead
//    for (;;)
//    {
//        if (next_block_size==0)
//        {
//            if (pTcpSocket->bytesAvailable() < sizeof(quint32)) //если принято менее двух байтов (не принята общая длина сообщения)
//            {
//                break;
//            }
//            in >> next_block_size;
//        }

//        if (pTcpSocket->bytesAvailable() < next_block_size)
//        {
//            break;
//        }

//        //QTime time;
//        //QString str;
//        //in >> time >> str;
//        //pTxtInfo->append(time.toString() + " " + str);

//        qint32 in_byte;
//        for (qint32 i=0; i<=next_block_size; i++)
//        {
//            in>>in_byte;
//            pTxtInfo->append(QString::number(in_byte) +"r");
//        }


//        next_block_size = 0;
//    } //for




//timerSend->start(1000);





//    QFile file(":/Style/style_1.qss");
//    file.open(QFile::ReadOnly);
//    QString str_style = QLatin1String(file.readAll());
//    this->setStyleSheet(str_style);
