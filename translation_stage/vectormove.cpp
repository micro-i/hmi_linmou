#include "vectormove.h"
#include <QDebug>

QSerialPort vectorMove::serial;

vectorMove::vectorMove()
{

}

int vectorMove::SetComPortn(int port_number)
{
    serial.setPortName(QString("COM%1").arg(port_number));
    serial.setBaudRate(QSerialPort::Baud57600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    return serial.open(QIODevice::ReadWrite);
}

void vectorMove::CloseComPort()
{
    serial.close();
}

void vectorMove::SetSpeed(int Channel, int Speed)
{
    QString order;
    switch (Channel) {
    case 0:
        order = "SX";
        break;
    case 1:
        order = "SY";
        break;
    case 2:
        order = "SZ";
        break;
    case 3:
        order = "SL";
        break;
    default:
        return;
    }
    order += QString::number(Speed) + ";";
    executeRequest(order);
}

void vectorMove::SetDistance(int Channel, int Distance)
{
    QString order;
    switch (Channel) {
    case 0:
        order = "DX";
        break;
    case 1:
        order = "DY";
        break;
    case 2:
        order = "DZ";
        break;
    case 3:
        order = "DL";
        break;
    default:
        return;
    }
    order += QString::number(Distance) + ";";
    executeRequest(order);
}

void vectorMove::SetMaxSpeed(int Channel, int MaxSpeed)
{
    QString order;
    switch (Channel) {
    case 0:
        order = "MX";
        break;
    case 1:
        order = "MY";
        break;
    case 2:
        order = "MZ";
        break;
    case 3:
        order = "ML";
        break;
    default:
        return;
    }
    order += QString::number(MaxSpeed);
    executeRequest(order);
}

QString vectorMove::GetDisplay(int Channel)
{
    QString order;
    switch (Channel) {
    case 0:
        order = "UX;";
        break;
    case 1:
        order = "UY;";
        break;
    case 2:
        order = "UZ;";
        break;
    case 3:
        order = "UL;";
        break;
    default:
        return QString();
    }
    executeRequest(order);
    serial.waitForReadyRead(3000);
    QByteArray buf = serial.readAll();
    return QString(buf);
}

int vectorMove::GetStatus(int Content)
{
    QString order = "UM;";
    if(!executeRequest(order)){
        return 0;
    }
    serial.waitForReadyRead(3000);
    QByteArray buf = serial.readAll();
    return (buf.data()[0] == 0x0);
}

void vectorMove::PrepareArbitrarySpeedData(int Channel, int total_seg, int duration[], int acceleration[], int repeat)
{
    QString order;
    QString side;
    switch (Channel) {
    case 0:
        order = "NX";
        side = "RX";
        break;
    case 1:
        order = "NY";
        side = "RY";
        break;
    case 2:
        order = "NZ";
        side = "RZ";
        break;
    case 3:
        order = "NL";
        side = "RL";
        break;
    default:
        return;
    }
    order += QString::number(total_seg) + ";";
    QString taccele;
    for(int i=0; i<total_seg; i++){
        order += "LT" + QString::number(duration[i]) + ";";
        order += "LA" + QString::number(acceleration[i]) + ";";
    }
    order += side + QString::number(repeat) + ";";
    executeRequest(order);
}

void vectorMove::TriggerArbitrarySpeed(int Channel)
{
    QString order = "TM" + QString::number(Channel) + ";";
    executeRequest(order);
}

void vectorMove::Stop()
{
    executeRequest("PA;");
}

void vectorMove::RecallSaveProcedures(int RecallSave, int ProNum)
{

}

bool vectorMove::executeRequest(QString order)
{
    qDebug() << "ready: " << order << endl;
    int idx = 0;
    for(auto& ch : order){
        char c = ch.toLatin1();
        serial.write(&c, 1);
        serial.waitForReadyRead();
        QByteArray buf = serial.readAll();
        if(buf.toHex() != "0d"){
            qDebug() << "Execute failed : " << idx << endl;
            return false;
        }
        idx++;
    }
    qDebug() << "end: " << order << endl;
    return true;
}
