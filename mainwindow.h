#pragma once

#include <QMainWindow>
#include <QSerialPort>
#include <QSettings>
#include <QVariant>
#include <QTcpSocket>
#include <QSctpSocket>
#include <QString>
#include <QRegExp>
#include <QThread>
#include <QCloseEvent>
#include <QAction>
#include <QFile>
#include <iostream>

#include "string.h"
#include "windows.h"
#include "conio.h"
#include "inc/l502api.h"

#include "init10A.h"
#include "L502.h"
#include "Generators.h"

class InitGens;
class initial10A;
class L502init;
class ChangeSampleRateForm;

QT_BEGIN_NAMESPACE
namespace Ui { class DeviceManager; }
QT_END_NAMESPACE


class DeviceManager : public QMainWindow
{
    Q_OBJECT

public:
    DeviceManager(QWidget *parent = nullptr);
    ~DeviceManager();

private:
    Ui::DeviceManager* ui;    //_____указатель на интерфейс

    QThread* thInit10A; //поток выполнения инициализации 10А
    QThread* thInitL502; //поток выполнения инициализации L502 devices
    QThread* thInitGens; //поток выполнения инициализации генераторов

    InitGens* objInitGens;
    L502init* devL502;
    initial10A* dev10A;

    ChangeSampleRateForm* m_form=nullptr;

    HANDLE m_hSerial=nullptr;

private slots:
    void slotDraw(QString msg); //отрисовка виджета из потока
};

