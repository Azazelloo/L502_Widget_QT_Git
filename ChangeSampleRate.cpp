#include "ChangeSampleRate.h"
#include "ui_ChangeSampleRateDialog.h"
#include <QDebug>

ChangeSampleRateForm::ChangeSampleRateForm(QWidget* parent,QVector<QTcpSocket*>* v_sockets):QDialog(parent),m_ui(new Ui::ChangeSampleRateForm),m_v(v_sockets){
    m_ui->setupUi(this);
    connect(m_ui->ChangeSampleRateButton,&QPushButton::clicked,this,[=](){
        QTcpSocket* thSock;
        for(auto& sock:*m_v){
            thSock=new QTcpSocket(this);
            thSock->setSocketDescriptor(sock->socketDescriptor());
            thSock->write(QString("FUNCtion:ARBitrary:SRATe "+m_ui->newSampleRate->text()+"\n").toUtf8());
            thSock->waitForBytesWritten(1); //минимальная задержка для гарантии отправки сообщения
        }
    });

}

ChangeSampleRateForm::~ChangeSampleRateForm(){

    delete m_ui;
    m_ui=nullptr;
    m_v=nullptr;
}
