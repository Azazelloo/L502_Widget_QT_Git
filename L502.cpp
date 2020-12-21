#include "L502.h"

L502init::L502init(QSettings* sett):create(nullptr){
    //читаем названия ldr файлов из ini файла
    create=new t_l502_hnd();

    nameLdrFile1=sett->value("blackfin/dev1", "error").toString();
    nameLdrFile2=sett->value("blackfin/dev2", "error").toString();
}

L502init::~L502init(){
    X502_Close(*create);
    delete create;
}

//______получаем обработчик L502, устанавливаем соединение
bool L502init::connect_L502(t_l502_hnd* create){

    int num_l502;
    uint32_t counter;
    char serials[5][32];
    L502_GetSerialList(serials, 2, 0, &counter);
    if (counter!=0){
        for (size_t i=0;i<counter;i++){
            qDebug()<< serials[i] << "->[" << i << "]" <<endl;
        }
        //std::cout << "Enter number L502->";
        num_l502 = 0;

        if (num_l502>=0 && num_l502 <=9){
            int err = L502_Open(*create, serials[num_l502]);
            if (err != 0){
                qDebug()<< "----Error connection!" <<endl;
                return false;
            }
            else{
                qDebug()<< "----Connection established!" <<endl;
                return true;
            }
        }
        else{
            qDebug()<< "----Input  error!" <<endl;
            return false;
        }
    }
    else{
        qDebug()<< "L502 not found!" <<endl;
        return false;
    }
}


void L502init::connectL502devices(){
    int err=0;
    *create = X502_Create();
    if(connect_L502(create)){
        emit draw(QString("L502[0] connected!"));

        std::string pathToLdr1("bf/");
        pathToLdr1+=nameLdrFile1.toStdString();

        err = X502_BfLoadFirmware(*create, pathToLdr1.c_str());
        if (err != X502_ERR_OK){
            emit draw(QString("---Erorr: blackfin[0] load error"));
        }
        else{
            emit draw(QString("blackfin[0] loaded!"));
        }

    }
    else{
        emit draw(QString("---Error: L502[0] not connected!"));
    }

    emit finished();
}
