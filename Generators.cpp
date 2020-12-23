#include "Generators.h"

//QVector<QTcpSocket*> DeviceManager::v_sockets;

InitGens::InitGens(QSettings* sett):m_sett(sett),m_pTcpSocket(nullptr){
    //читаем адреса генераторов из ini файла
    if(m_sett!=nullptr){
        addresses.push_back(m_sett->value("addresses/gen1", "error").toString());
        addresses.push_back(m_sett->value("addresses/gen2", "error").toString());
        addresses.push_back(m_sett->value("addresses/gen3", "error").toString());
        addresses.push_back(m_sett->value("addresses/gen4", "error").toString());
    }
}

InitGens::~InitGens(){
    qDebug()<<"Destructor InitGens!\n";

    m_sett=nullptr;
    delete m_pTcpSocket;

    for(auto& sock:v_sockets_descriptors){
        sock->disconnectFromHost();
        sock->close();
        delete sock;
        sock=nullptr;
    }

}


void InitGens::GensSet(QSettings* sett){
    //читаем настройки генераторов из ini файла
        m_sett=sett;
        numSeq=new QString(m_sett->value("settings/numSeq", "error").toString());
        freqSeq=new QString(m_sett->value("settings/freqSeq", "error").toString());
        ampSignal=new QString(m_sett->value("settings/ampSignal", "error").toString());
        countArbs=new int(m_sett->value("settings/countArbs", "error").toInt());
}

//_____проверяем валидность ip адреса,запускаем инициализацию
void InitGens::CheckAddrAndRunInit(){
    QRegExp checkIpAddress("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}"); //регулярное выражение для проверки формата IP адреса

    this->GensSet(new QSettings("ini/settings.ini", QSettings::IniFormat));//подтягиваем актуальные настройки

    for(auto& addr:addresses){
        if(checkIpAddress.exactMatch(addr)){
            InitialGen(&addr);
        }
        else{
            emit draw(QString("---Error: check the address format!"));
        }
    }
    emit finished();
}

//_____создаем в генераторе arb файл используя arb файл на хосте
void InitGens::CreateArbFileInGenMem(size_t counter){
    QFile file("datas/FM"+QString().setNum(counter)+".arb"); //читаем файл на хосте
    if (!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Arb file is not open!"<<endl;
    }
    QString data,sizeData,sizeOfSize;
    data = file.readAll();

    sizeData.setNum(data.size()); //размер arb файла
    sizeOfSize.setNum(sizeData.size()); //размер РАЗМЕРА arb файла (так надо)

    m_pTcpSocket->write(QString("MMEMory:DOWNload:FNAMe \"INT:\\MyArbs\\arb"+QString().setNum(counter)+".arb\"\n").toUtf8());
    m_pTcpSocket->write(QString("MMEMory:DOWNload:DATA #"+sizeOfSize+sizeData+data+"\n").toUtf8());

    file.close();
}

//_____тестим загрузку своей seq последовательности
void InitGens::CreateCustomSeq(){

    QFile file("datas/FM.seq");
    if (!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Arb file is not open!"<<endl;
    }
    QString data,sizeData,sizeOfSize;
    data = file.readAll();

    sizeData.setNum(data.size());
    sizeOfSize.setNum(sizeData.size());

    m_pTcpSocket->write(QString("MMEMory:DOWNload:FNAMe \"INT:\\MyArbs\\FM.seq\"\n").toUtf8());
    m_pTcpSocket->write(QString("MMEMory:DOWNload:DATA #"+sizeOfSize+sizeData+data+"\n").toUtf8());

    file.close();
}

//_____создаем seq файл из загруженных arb файлов
void InitGens::CreateSeqFromArb(){
    //m_pTcpSocket->write(QString("DATA:VOLatile:CLEar\n").toUtf8()); //очищает память сигнала

    QString sendStr("\"seqFm\"");
    QString sizeData,sizeOfSize;

    for(int i=1;i<=*countArbs;i++){
        m_pTcpSocket->write(QString("MMEMory:LOAD:DATA \"INT:\\MyArbs\\arb"+QString().setNum(i)+".arb\"\n").toUtf8());//подгружаем arb файлы
        sendStr+=",\"INT:\\MyArbs\\arb"+QString().setNum(i)+".arb\",1,\"onceWaitTrig\",\"maintain\",0"; //пишем seq последовательность
    }

    sizeData.setNum(sendStr.size());
    sizeOfSize.setNum(sizeData.size());
    m_pTcpSocket->write(QString("DATA:SEQuence #"+sizeOfSize+sizeData+sendStr+"\n").toUtf8());//создаем seq файл
    m_pTcpSocket->write(QString("FUNCtion:ARBitrary \"seqFm\"\n").toUtf8());//делаем seq файл активным
    m_pTcpSocket->write(QString("MMEMory:STORe:DATA \"INT:\\MyArbs\\seqFm.seq\"\n").toUtf8());// сохрняем seq файл в память

}

void InitGens::InitialGen(QString* addr){

    m_pTcpSocket=new QTcpSocket(this);

    connect(m_pTcpSocket, &QTcpSocket::connected,this,[&](){

        //_____прием данных с генератора
        connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

        emit draw(QString("---"+ *addr +" connected!"));

        //_____записываем SCPI команды
        m_pTcpSocket->write(QString("*IDN?\n").toUtf8()); //запрашиваем индентификатор генератора
        m_pTcpSocket->waitForReadyRead(5000);//ожидаем ответ от сокета

        m_pTcpSocket->write(QString("*CLS\n").toUtf8()); // сбрасываем состояние

        m_pTcpSocket->write(QString("*RST\n").toUtf8()); // reset

        m_pTcpSocket->write(QString("TRIGger:SOURce EXTernal\n").toUtf8());//настройка тригера как внешний

        m_pTcpSocket->write(QString("TRIGger:SLOPe POSitive\n").toUtf8());// фронт

        m_pTcpSocket->write(QString("DATA:VOLatile:CLEar\n").toUtf8());

        //m_pTcpSocket->write(QString("MMEMory:MDIRectory \"MyArbs\"\n").toUtf8()); //ПРОВЕРИТЬ СУЩЕСТВОВАНИЕ ДИРЕКТОРИИ ИЛИ ЗАПРОСИТЬ РАЗРЕШЕНИЕ

        m_pTcpSocket->write(QString("DATA:ARB2:FORM AABB\n").toUtf8());

        //_____грузим arb файлы в память генератора
        for(int i=1;i<=*countArbs;i++){
            CreateArbFileInGenMem(i);
        }

        CreateSeqFromArb(); //создаем seq файл из загруженных arb файлов
        CreateCustomSeq();  //создаем свой seq файл

        m_pTcpSocket->write(QString("MMEMory:LOAD:DATA \"INT:\\MyArbs\\seqFm.seq\"\n").toUtf8());//загружаем сгенерированную последовательность
        m_pTcpSocket->write(QString("FUNCtion:ARBitrary \"INT:\\MyArbs\\seqFm.seq\"\n").toUtf8()); //делаем сгенерированную последовательность активной

//        m_pTcpSocket->write(QString("MMEMory:LOAD:DATA \"INT:\\MyArbs\\FM.seq\"\n").toUtf8());//загружаем custom последовательность
//        m_pTcpSocket->write(QString("FUNCtion:ARBitrary \"INT:\\MyArbs\\FM.seq\"\n").toUtf8()); //делаем custom последовательность активной


        m_pTcpSocket->write(QString("FUNCtion:ARBitrary:SRATe "+*freqSeq+"\n").toUtf8()); //устанавливаем частоту дискретизации последовательности

        m_pTcpSocket->write(QString("FUNCtion ARB\n").toUtf8());

        m_pTcpSocket->write(QString("OUTP1 1\n").toUtf8());

        m_pTcpSocket->write(QString("SOURce1:VOLTage "+*ampSignal+"\n").toUtf8());

         m_pTcpSocket->write(QString(":FUNC:ARB:FILT OFF\n").toUtf8()); // отключаем фильтр

        //проверяем наличие ошибок на генераторе
        m_pTcpSocket->write(QString("SYST:ERR?\n").toUtf8());
        m_pTcpSocket->waitForReadyRead(5000);//ожидаем ответ от сокета

        if(error!="+0,\"No error\""){
            emit draw(QString("---Error: generator setting  error!"));
        }
        else{
            emit draw(QString("---Generator setting completed!"));
        }

        //emit finished();
    });

    //_____обработчик ошибок соединения с сокетом
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(slotError(QAbstractSocket::SocketError))
            );

    //_____попытка подключения к генератору
    m_pTcpSocket->connectToHost(*addr, 5025);
    if(!(m_pTcpSocket->waitForConnected(3000))){
        emit draw(QString("---Erorr: "+ *addr +" not connected!"));
    }
    else{
        v_sockets_descriptors.push_back(m_pTcpSocket);
    }
}

void InitGens::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                             "The host was not found." :
                             err == QAbstractSocket::RemoteHostClosedError ?
                                 "The remote host is closed." :
                                 err == QAbstractSocket::ConnectionRefusedError ?
                                     "The connection was refused." :
                                     QString(m_pTcpSocket->errorString())
                                     );
    emit draw(strError);
}

void InitGens::slotReadyRead()
{
    error= QString::fromUtf8(m_pTcpSocket->readAll().trimmed());
    emit draw(error);
}



