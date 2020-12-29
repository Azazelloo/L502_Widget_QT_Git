#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ChangeSampleRate.h"

void SetGenOutps(QVector<QTcpSocket*>* vSockets,QString state){
    QTcpSocket* thSock;
    for(auto& sock:*vSockets){
        thSock=new QTcpSocket();
        thSock->setSocketDescriptor(sock->socketDescriptor());
        thSock->write(QString("OUTP1 "+state+"\n").toUtf8());
        thSock->waitForBytesWritten(1); //минимальная задержка для гарантии отправки сообщения
    }
}

//_____фукнция, отправляющая два байта на COM
size_t Send2BytesOnCOM(QSerialPort* m_serial,unsigned short data){
    QByteArray msg;
    msg.push_back(static_cast<char>(data)); // первый байт слова
    msg.push_back(static_cast<char>(data>>8)); //второй байт слова

    size_t sendSize=m_serial->write(msg);
    m_serial->waitForBytesWritten(1); //минимальная задержка для гарантии отправки сообщения
    return sendSize;
}

DeviceManager::DeviceManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeviceManager)
    ,thInit10A(new QThread)
    ,thInitL502(new QThread)
    ,thInitGens(new QThread)
    ,objInitGens(nullptr)
    ,devL502(nullptr)
    ,dev10A(nullptr){

    ui->setupUi(this);

    QSettings* sett = new QSettings("ini/settings.ini", QSettings::IniFormat);

    //_____открываем соединени с микроконтроллером
    m_serial=new QSerialPort();
    //подтягиваем настройки из ini файлаaaa
            m_serial->setPortName("\\\\.\\"+QString(sett->value("COM settings/Port", "error").toString())); //port name
            m_serial->setBaudRate(sett->value("COM settings/BaudRate", "error").toInt()); //baud rate
            m_serial->setDataBits((QSerialPort::DataBits)(sett->value("COM settings/ByteSize", "error").toInt())); //byte size
            m_serial->setParity((QSerialPort::Parity)(sett->value("COM settings/Parity", "error").toInt())); //parity
            m_serial->setStopBits((QSerialPort::StopBits)(sett->value("COM settings/StopBits", "error").toInt())); //stop bits

            if (!(m_serial->open(QIODevice::ReadWrite))){
                qDebug()<<"Error open COM!"<<endl;
            }


    //_____ИНИЦИАЛИЗАЦИЯ ГЕНЕРАТОРОВ
    objInitGens=new InitGens(sett);

    objInitGens->moveToThread(thInitGens);
    connect(objInitGens,SIGNAL(draw(QString)),this,SLOT(slotDraw(QString)));

    connect(thInitGens,&QThread::started,objInitGens,&InitGens::CheckAddrAndRunInit,Qt::DirectConnection);

    connect(ui->changeSampleRate,&QPushButton::clicked,m_form=new ChangeSampleRateForm(this,objInitGens->GetSocketsVector()), &ChangeSampleRateForm::show);

    connect(ui->initGens,&QPushButton::clicked,this,[=](){
        thInitGens->start();
    });

    connect(objInitGens,&InitGens::finished,thInitGens,[=](){
        qDebug()<<"thInitGens -> exit"<<endl;
        ui->changeSampleRate->setEnabled(true);
        ui->SyncStm32->setEnabled(true);

        thInitGens->exit(0);
    });
    /*////////////////////////////////////*/

    //_____СИНХРОНИЗАЦИЯ STM32 И ГЕНЕРАТОРОВ

    connect(ui->SyncStm32,&QPushButton::clicked,this,[=](){

        auto vSockets=objInitGens->GetSocketsVector(); //вектор указателей на открытые сокеты

        SetGenOutps(vSockets,QString("OFF")); //выключаем выходы генераторов

        Send2BytesOnCOM(m_serial,0x8000); //команда переводящая микроконтроллер в режим ожидания

        SetGenOutps(vSockets,QString("ON"));//включаем выходы генераторов
        Sleep(300); //(!!!) задержка отсекающая неопределенное поведение генератора при включении каналов с синхро по внешнему триггеру

        Send2BytesOnCOM(m_serial,0x4000 + ui->targetDelay->value());//QString(ui->targetDelay->text()).toUInt()); //команда на reset микроконтроллера + выставление дальности до цели

        qDebug()<<"Synchronization is done!"<<endl;
    });

    /*////////////////////////////////////*/

    //_____ИНИЦИАЛИЗАЦИЯ L502 devices
    devL502=new L502init(sett);

    devL502->moveToThread(thInitL502);
    connect(devL502,SIGNAL(draw(QString)),this,SLOT(slotDraw(QString)));

    connect(thInitL502,&QThread::started,devL502,&L502init::connectL502devices,Qt::DirectConnection);
    connect(devL502,&L502init::finished,thInitL502,[=](){
        qDebug()<<"thInitL502 -> exit"<<endl;
        thInitL502->exit(0);
    });

    connect(ui->connectL502devices,&QPushButton::clicked,this,[=](){
        thInitL502->start();
    });
    /*////////////////////////////////////*/

    //_____ИНИЦИАЛИЗАЦИЯ 10А
    dev10A=new initial10A(ui);

    dev10A->moveToThread(thInit10A);
    connect(dev10A,SIGNAL(draw(QString)),this,SLOT(slotDraw(QString)));

    connect(thInit10A,&QThread::started,dev10A,&initial10A::run10A,Qt::DirectConnection);

    connect(dev10A,&initial10A::finished,thInit10A,[=](){
        qDebug()<<"thInit10A -> exit"<<endl;
        thInit10A->exit(0);
    });

    connect(ui->init10A,&QPushButton::clicked,this,[=](){
        thInit10A->start();
    });
    /*////////////////////////////////////*/
}

DeviceManager::~DeviceManager(){

    delete ui;

    thInit10A->exit(0);
    delete thInit10A;

    thInitL502->exit(0);
    delete thInitL502;

    thInitGens->exit(0);
    delete thInitGens;

    m_serial->close();
    delete m_serial;
}

void DeviceManager::slotDraw(QString msg){
    ui->textBrowser->append(msg);
}



