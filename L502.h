#pragma once
#include "mainwindow.h"
#include "ui_mainwindow.h"

class L502init:public QObject{
    Q_OBJECT

private:
    t_l502_hnd* create;
    QString nameLdrFile1;
    QString nameLdrFile2;
public:
    L502init(QSettings* sett);
    ~L502init();

    bool connect_L502(t_l502_hnd* create);
    void connectL502devices();

signals:
    void finished();
    void draw(QString msg);
};
