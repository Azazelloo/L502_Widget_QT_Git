#include "init10A.h"
#include "inc/WDMTMKv2.cpp"


initial10A::initial10A(Ui::DeviceManager* ui):bcnum(0),addrOU(1),GoodRNVULC(0xF000),GoodRN10H(0x001E),m_ui(ui){
    //hBcEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//создаем событие для прерываний
    tmkEvD=new TTmkEventData;
}

initial10A::~initial10A(){

    m_ui=nullptr;

    tmkdone(ALL_TMKS);
    TmkClose();
    CloseHandle(hBcEvent);
}

int initial10A::Init10A() {
    int err = 0;
    bcnum=0;
    addrOU=1;

    if (!TmkOpen()) {
        qDebug()<< "---TmK is open!" <<endl;
        emit draw(QString("---TmK is open!"));
    }
    else {
        qDebug()<< "---Error: TmK is not open!" << endl;
        emit draw(QString("---Error: TmK is not open!"));
    }

    err = tmkconfig(bcnum);

    err = bcreset(); // обнуляем биты

    hBcEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//создаем событие для прерываний

    tmkdefevent(hBcEvent, TRUE); //устанавливаем событие для прерываний
    ResetEvent(hBcEvent);
    err = bcdefirqmode(0x00000000); //все прерывания разрешены

    if (!err) {
        qDebug()<< "---Setting complete!" << endl;
        emit draw(QString("---Setting complete!"));
    }
    else {
        qDebug()<< "---Error: setting not complete!" <<endl;
        emit draw(QString("---Error: setting not complete!"));
    }

    //______Инициализация 10А
    uint16_t sendRcv[16] = { 0 };
    err = OUtoKK(sendRcv, 5, 1,2); //проверка экспресс диагностики
                                   //начинаем читать со 2 слова
    if (!err && sendRcv[0] == 1) {
        qDebug()<< "---Express diagnostic complete!" <<endl;
        emit draw(QString("---Express diagnostic complete!"));
    }
    else {
        qDebug()<< "---Error: express diagnostic not complete!" <<endl;
        emit draw(QString("---Error: express diagnostic not complete!"));
    }

    //команда на инициализацию
    sendRcv[0] = 0x01F5;
    err = KKtoOU(sendRcv, 4, 1);
    qDebug()<< "Waiting initialization..." <<endl;
    emit draw(QString("Waiting initialization..."));
    Sleep(4000);//ожидание после инициализации не менее 3х секунд

    if (!err) {
        addrOU = 12; //согласно протоколу, адрес прибора меняется на 14(в восьмеричной системе)
        for (int i = 0; i < 5; i++) {
            OUtoKK(sendRcv, 2, 2,2); //начинаем читать со 2 слова
            if ((sendRcv[0] & GoodRN10H) && (sendRcv[1] & GoodRNVULC)) {
                qDebug()<< "---Initialization complete!" <<endl;
                emit draw(QString("---Initialization complete!"));
                break;
            }
            else {
                qDebug()<< "---Error: initialization not complete!" <<endl;
                emit draw(QString("---Error: initialization not complete!"));
                Sleep(1000);
            }
        }
    }
    else {
        qDebug()<< "---Error: initialization not complete!" <<endl;
        emit draw(QString("---Error: initialization not complete!"));
    }

    return err;
}


void initial10A::run10A(){

    if(!Init10A()){//инициализируем 10А
        //_____считываем из файла массив для отправки
            std::ifstream in("ini/words.txt"); //если считать не удастся отправится массив 0
            if (in.is_open()) {
                qDebug()<< "---Read words from txt complete!" <<endl;
                for (int i = 0; i < VALFORM1; i++) {
                    in >>std::hex>> dataExchange[i];
                }
            }
            else {
                qDebug()<< "---Read words from txt not complete!" <<endl;
            }
            in.close();

        //_____обзоры
           //addrOU=12;

           double wA = 0, wCK = 0;
           int revCounter=0;

           std::ofstream out;
           out.open("ini/disp.txt"); //сюда пишем логи обзоров

        //_______инициализация приводов
           uint16_t tmpRK4 = dataExchange[3];
           dataExchange[3] ^= 0xA800; //оставляем только привод и порог1
           SingleExchange();

           dataExchange[3] = tmpRK4;

           while(revCounter< m_ui->countReviews->value()){

               if (!SingleExchange()){

                   //____анализ угла отклонения
                     wA= static_cast<int16_t>(dataExchangeRet[1])*CMR; //угол отклонения луча антенны в горизонтальной плоскости
                     wCK=static_cast<int16_t>(dataExchangeRet[2])*CMR; //угол сканирования

                     if ((wA <= -39) && (!(dataExchange[3] & 0x1000))) { //меняем направление движения антенны при достижении крайнего положения
                         out << "Review: "<<revCounter<<"\n";
                         ++revCounter;
                         GetReview(out);
                         dataExchange[3] ^=(unsigned short) 0x1000;
                     }
                     if ((wA >= 39) && (dataExchange[3] & 0x1000)) { //меняем направление движения антенны при достижении крайнего положения
                         out << "Review: " << revCounter << "\n";
                         ++revCounter;
                         GetReview(out);
                         dataExchange[3] ^=(unsigned short) 0x1000;
                      }
                      /*/////////////////////////////*/
               }
               else{
                   qDebug()<< "---Continuous exchange error!\n";
                   out.close();
                   break;
               }

           }

    }

    tmkdone(ALL_TMKS);
    TmkClose();
    CloseHandle(hBcEvent);

    emit finished();
}


int initial10A::OUtoKK(uint16_t* word,unsigned short subAddr,unsigned short numWords,unsigned short startWord) {
    bcdefbase(0);
    bcputw(0, CW(addrOU, RT_TRANSMIT, subAddr, numWords));
    bcstart(0, DATA_RT_BC);

    WaitForSingleObject(hBcEvent, INFINITE); //ждем прерывания
    bcgetblk(startWord, word, numWords); // читаем со второго слова
    ResetEvent(hBcEvent);// обновляем событие

    tmkgetevd((TTmkEventData*)tmkEvD);
    if (((TTmkEventData*)tmkEvD)->bc.wResult == 0) {  //при успешном обмене в  bc.wResult будет 0
        return 0;
    }

    return 1;
}

int initial10A::KKtoOU(uint16_t* word, unsigned short subAddr, unsigned short numWords) {
    bcdefbase(0);
    bcputw(0, CW(addrOU, RT_RECEIVE, subAddr, numWords));
    bcputblk(1, word, numWords);
    bcstart(0, DATA_BC_RT);

    WaitForSingleObject(hBcEvent, INFINITE); //ждем прерывания
    ResetEvent(hBcEvent);

    tmkgetevd((TTmkEventData*)tmkEvD);
    if (! ((TTmkEventData*)tmkEvD)->bc.wResult) { //при успешном обмене в  bc.wResult будет 0
        return 0;
    }

    return 1;
}

int initial10A::SingleExchange() {
    int err = 0;
    err = KKtoOU(dataExchange, 2, VALFORM1); //записываем 17 слов во второй подадрес

    err = KKtoOU(&freqWord, 1, 1); //передаем сигнал тактовой частоты

    err = OUtoKK(dataExchangeRet, 3, VALFORM2,2); //принимаем 25 слов с третьего подадреса
                                                  //начинаем читать со 2 слова

    Sleep(1); //без задержки обмен с манчестером зависает -> разобраться (зависимость от компилятора?)
    return err;
}

//_____логируем результаты обзоров
bool initial10A::GetReview(std::ofstream& out) {

    int err = 0;
    unsigned short subAddr = 4;
    uint16_t countForms = dataExchangeRet[10]; // считываем количество формуляров целей
    uint16_t tmpDataExchange[31];

    if (countForms) { //если есть формуляры

        std::vector<uint16_t> formsReviews;

        for (int i = 0; i < ((dataExchangeRet[10]*3)/31) ;i++) { //считаем сколько нужно прочитать полных подадресов исходя из количества целей
            err = OUtoKK(tmpDataExchange, subAddr, 31, 2);
            ++subAddr; //переходим на следующий подадрес
            formsReviews.insert(formsReviews.end(), tmpDataExchange, tmpDataExchange+31);
        }
        //считываем последний неполный подадрес
        uint16_t countRem = (countForms * 3) - 31 * ((countForms * 3) / 31); //считаем сколько слов не считано
        err = OUtoKK(tmpDataExchange, subAddr, countRem, 2);
        formsReviews.insert(formsReviews.end(), tmpDataExchange, tmpDataExchange + countRem);

        //_____пишем логи целей
        for (int i = 0; i < countForms;i+=3) {
            out << std::setw(5) << i/3 << "\t\t";
            out << std::setw(10) << ((formsReviews[i]) >> 4)*kvant + dataExchange[5] << "\t"; //дальность -> сдвинутое слово умножаем на квант + дальность начала зоны обнаружения
            out << std::setw(10)<<(short)formsReviews[i+1] * CMR / 2 << "\t"; //угол начала
            out << std::setw(10)<<(short)formsReviews[i+2] * CMR / 2 << "\t"; //угол конца
            out << "\n";
        }
        out << "\n";

        return true;
    }
    return false;
}



