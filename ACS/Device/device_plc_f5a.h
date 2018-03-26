#ifndef DEVICE_PLC_F5A_H
#define DEVICE_PLC_F5A_H

#include <QWidget>
#include <QTcpSocket>
#include <QtWidgets>
#include "EXT/ext.h"

class QTextEdit;
class QLineEdit;



//??----------------------------------------------------------------
struct ModeStruct       //структура "Режим", описывающая конкретный режим работы данного устройства. Из этих структур составляется список режимов работы устроства
{
    qint16 number;
    QString name;
    QByteArray settings;
    QBitArray params;
};


//struct ConfigStructFull       //структура "Конфигурация", описывающая параметры, поступающие от устройства
//{
//    qint32 param_number;
//    QString coefficient;
//    QString unit;
//    QString param_name;
//    qint32 param_adress;
//};
//??----------------------------------------------------------------




class device_PLC_F5A : public QWidget
{
    Q_OBJECT

private:
    //входные параметры
    //QQueue <QPair <qint32, QPair <QString, qint32> > >* p_queue_to_db;
    //QVector <QPair <qint32, QPair <QString, QString> > >* p_config_to_db;

    //указатель на очередь к БД
    QQueue <structDataFlow> *p_queue_to_db;
    //указатель на флаг записи в очередь
    bool *flgWriteToQu;

    QString strHost;
    int nPort;

    //объекты TCP
    QTcpSocket* p_tcp_socket;
    QTimer* p_timer_connect;                    //таймер, по сигналу которого происходят попытки подключения к ПЛК
    QTimer* p_timer_send;
    qint32 next_block_size;

    //интерфейс
    QTextEdit* p_txt_info_diag_buffer;          //текстовое поле диагностического буфера
    QLineEdit* p_line_info_addr;
    QLineEdit* p_line_info_port;
    QListWidget* lst_wdg;                       //список чекбоксов параметров устройства
    //del
    QCheckBox* p_chk_box;

    //служебные функции
    QString diag_string (QString str);          //создание строки диагностического сообщения

    //режимы (??)
    QList <ModeStruct> mode_list;

    //конфигурация параметров
    QVector <ConfigStruct> config_vector;
    //заполнение конфигурации параметров
    void fillConfigVector();



public:
    device_PLC_F5A(QWidget* pwgt = 0, QQueue <structDataFlow> * p_queue_to_db_in=0, bool *flgWriteToQu=0
                   /*QVector <QPair <qint32, QPair <QString, QString> > >* p_config_to_db_in=0, bool* glob_flag=0*/ ) ;

    QVector <ConfigStruct> getConfigVector ();



protected:




private slots:

    //TCP клиент
    void slot_ready_read ( );
    void slot_send_to_server();
    void slot_connected ( );
    void slot_disconnected();
    void slot_error(QAbstractSocket::SocketError);
    void timer_send_tick();
    void timer_connect_tick();

    void addr_verify(QString);
    //void slotError (QAbstractSocket::SocketError);
    //void timerStartStop();

};

#endif // DEVICE_PLC_F5A_H








//private конструктора
//    QLineEdit* p_txt_input;         //поле ввода информации для передачи
//    QCheckBox* p_chk_box;           //чекбокс включения передачи по таймеру
