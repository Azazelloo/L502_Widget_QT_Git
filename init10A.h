#pragma once
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QObject>
#include "windows.h"


class initial10A:public QObject{
    Q_OBJECT
private:
    size_t bcnum;
    size_t addrOU;
    uint16_t GoodRNVULC;
    uint16_t GoodRN10H;
    uint16_t freqWord = 0x0FFF; //частота
    HANDLE hBcEvent;
    void* tmkEvD;

    uint16_t dataExchange[VALFORM1] = { 0 };
    uint16_t dataExchangeRet[VALFORM2] = { 0 };

signals:
    void finished();
    void draw(QString msg);

public:
    initial10A();
    ~initial10A();

    int Init10A();
    int SingleExchange();
    bool GetReview(std::ofstream& out);
    int OUtoKK(uint16_t* word,unsigned short subAddr,unsigned short numWords,unsigned short startWord);
    int KKtoOU(uint16_t* word, unsigned short subAddr, unsigned short numWords);
    void run10A();
};

