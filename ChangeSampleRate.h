#pragma once
#include "mainwindow.h"
#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ChangeSampleRateForm; }
QT_END_NAMESPACE

class ChangeSampleRateForm:public QDialog{
    Q_OBJECT
private:
    Ui::ChangeSampleRateForm* m_ui = nullptr;
    QVector<QTcpSocket*>* m_v;
public:
    ChangeSampleRateForm(QWidget* parent=nullptr,QVector<QTcpSocket*>* v=nullptr);
    ~ChangeSampleRateForm();
};
