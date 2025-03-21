#ifndef VECTORMOVE_H
#define VECTORMOVE_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class vectorMove : public QObject
{
    Q_OBJECT
public:
    vectorMove();
    // 打开端口
    //port_number=1:com1;
    //port_number=2:com2;
    int SetComPortn(int port_number);
    // 关闭端口
    void CloseComPort();
    // 设置速度
    //Channel = 0,1,2,3 = X,Y,Z,L chaneel
    //Speed =-4096~4095
    void SetSpeed(int Channel, int Speed);
    // 设置距离
    //Channel = 0,1,2,3 = X,Y,Z,L channel
    //Distance = -2^30~2^30-1
    void SetDistance(int Channel, int Distance);
    // 设置最大速度
    //Channel = 0,1,2,3 = X,Y,Z,L
    //MaxSpeed =-4096~4095
    void SetMaxSpeed(int Channel, int MaxSpeed);
    // 获取显示位置
    //Channel = 0,1,2,3 = X,Y,Z,L
    QString GetDisplay(int Channel);
    // 获取状态
    //Content = 0/1/2 = stage/joint status/home
    int GetStatus(int Content);
    // 准备任意速度运行
    // Channel: X,Y,Z,L=0,1,2,3
    //Total_Seg: Total number of line segments. 1-680 for only using X Channel;1~340 for only using X and Y  Channel;
    //											1-170 for using X,Y,Z Channel or X,Y,Z,L Channel
    //duration:  Number of time slot. one time slot is equal 0.4096ms.Range : 1-2^30-1
    //acceleration: Number of acceleration units, One acceleration unit is equal to 360/(256*0.4096e-3)^2 degree/s^2.
    //repeat: repeat time
    void PrepareArbitrarySpeedData(int Channel, int total_seg, int duration[700], int acceleration[700], int repeat);
    // 任意速度运行
    // channels: Sum of Channels weight
    // 1 for X channel weight;
    // 2 for Y channel weight;
    // 4 for Z channel weight;
    // 8 for L channel weight;
    void TriggerArbitrarySpeed(int Channel);
    // 立即停止
    void Stop(void);
    //RecallSave=0/1=Recall/Save arbitrary speed procedure
    void RecallSaveProcedures(int RecallSave, int ProNum);
private:
    bool executeRequest(QString request);

    QSerialPort serial;
};

#endif // VECTORMOVE_H
