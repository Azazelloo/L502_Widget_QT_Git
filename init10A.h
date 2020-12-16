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
    HANDLE hBcEvent;
    void* tmkEvD;

signals:
    void finished();
    void draw(QString msg);

public:
    initial10A();
    ~initial10A();

    int OUtoKK(uint16_t* word,size_t subAddr,size_t numWords);
    int KKtoOU(uint16_t* word, size_t subAddr, size_t numWords);
    void run10A();
};

