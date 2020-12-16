#include "init10A.h"
#include "inc/WDMTMKv2.cpp"


initial10A::initial10A():bcnum(0),addrOU(0),GoodRNVULC(0xF000),GoodRN10H(0x001E){
    hBcEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//создаем событие для прерываний
    tmkEvD=new TTmkEventData();
}

initial10A::~initial10A(){
    CloseHandle(hBcEvent);
    delete tmkEvD; //удаление void указателя насколько корректно если тип переопределялся?
}


void initial10A::run10A(){

    //_______Предварительная настройка Tmk

    int err=0;
    addrOU=1;

    if (!TmkOpen()) {
        qDebug()<<"---TmK is open!" <<endl;

        err=tmkconfig(bcnum);
        err=bcreset(); // обнуляем биты

        tmkdefevent(hBcEvent, TRUE); //устанавливаем событие для прерываний
        ResetEvent(hBcEvent);
        err=bcdefirqmode(0x00000000); //все прерывания разрешены

        if(!err){
            emit draw(QString("Tmk setting complete!"));

            //______Инициализация 10А
            unsigned short sendRcv[16];

            err=OUtoKK(sendRcv,5,1); //проверка экспресс диагностики

            if (!err && sendRcv [0]==1) {
                emit draw(QString("Express diagnostic complete!"));
            }
            else {
                emit draw(QString("---Error: express diagnostic not complete!"));
            }
            //команда на инициализацию
            sendRcv[0] = 0x01F5;
            err=KKtoOU(sendRcv,4,1);

            qDebug() << "Waiting initialization..." <<endl;
            QThread::sleep(4);//ожидание после инициализации не менее 3х секунд

            if(!err){
                addrOU = 12; //согласно протоколу, адрес прибора меняется на 14(8)
                for (int i = 0; i < 5;i++) { //пытаемся получить состояние инициализации
                    err=OUtoKK(sendRcv, 2, 2);
                    if (!err && (sendRcv[0] & GoodRN10H) && (sendRcv[1] & GoodRNVULC)) {

                        emit draw(QString("Initialization 10A complete!"));
                        break;
                    }
                    else {
                        emit draw(QString("---Error: initialization 10A not complete!"));
                    }

                    QThread::sleep(1);
                }
            }
            else{
                emit draw(QString("---Error: initialization 10A not complete!"));
            }

        }
        else{
            emit draw(QString("---Error: tmk setting not complete!"));
        }
    }
    else {
        emit draw(QString("---Error: Tmk not open!"));

        qDebug()<<"---Error: Tmk not open!"<<endl;
        tmkdefevent(hBcEvent, TRUE); //устанавливаем событие для прерываний
        ResetEvent(hBcEvent);
        bcdefirqmode(0x00000000); //все прерывания разрешены
    }

    tmkdone(ALL_TMKS);
    TmkClose();
    emit finished();
}


int initial10A::OUtoKK(uint16_t* word,size_t subAddr,size_t numWords) {
    bcdefbase(0);
    bcputw(0, CW(addrOU, RT_TRANSMIT, subAddr, numWords));
    bcstart(0, DATA_RT_BC);

    WaitForSingleObject(hBcEvent, INFINITE); //ждем прерывания
    bcgetblk(2, word, numWords); // читаем со второго слова
    ResetEvent(hBcEvent);// обновляем событие

    tmkgetevd((TTmkEventData*)tmkEvD);
    if (! ((TTmkEventData*)tmkEvD)->bc.wResult) {
        return 0;
    }

    return 1;
}

int initial10A::KKtoOU(uint16_t* word, size_t subAddr, size_t numWords) {
    bcdefbase(0);
    bcputw(0, CW(addrOU, RT_RECEIVE, subAddr, numWords));
    bcputblk(1, word, numWords);
    bcstart(0, DATA_BC_RT);

    WaitForSingleObject(hBcEvent, INFINITE); //ждем прерывания
    ResetEvent(hBcEvent);

    tmkgetevd((TTmkEventData*)tmkEvD);
    if (! ((TTmkEventData*)tmkEvD)->bc.wResult) {
        return 0;
    }

    return 1;
}



