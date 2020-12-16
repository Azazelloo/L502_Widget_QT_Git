#pragma once
#include "mainwindow.h"
#include "ui_mainwindow.h"

class InitGens:public QObject{
    Q_OBJECT

private:
    QSettings* m_sett;
    QTcpSocket* m_pTcpSocket; //_____сокет генератора
    QVector<QTcpSocket*> v_sockets_descriptors; //_____дескрипторы открываемых сокетов для передачи в другой поток

    QVector<QString> addresses;
    QString* numSeq;
    QString* freqSeq;
    QString* ampSignal;
    QString error;
    int* countArbs;

signals:
    void finished();
    void draw(QString msg);

private slots:
    void slotError(QAbstractSocket::SocketError err);
    void slotReadyRead();

public:
    InitGens();
    InitGens(QSettings* sett);
    void GensSet(QSettings* sett);
    ~InitGens();
    void CheckAddrAndRunInit();
    void InitialGen(QString* addr);
    void ChangeSampleRate();
    QVector<QTcpSocket*>* GetSocketsVector(){
        return &v_sockets_descriptors;
    }
    void CreateArbFileInGenMem(size_t counter);
    void CreateSeqFromArb();
    void CreateCustomSeq();
};
