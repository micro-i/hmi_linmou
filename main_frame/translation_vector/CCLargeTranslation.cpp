#include "CCLargeTranslation.h"

#include <QThread>
#include <QDebug>

#include "vectormove.h"

CCLargeTranslation::CCLargeTranslation() : QObject(nullptr)
{
    int ret = m_vectorMove.SetComPortn(4);
}

CCLargeTranslation::~CCLargeTranslation()
{
    m_vectorMove.CloseComPort();
}

void CCLargeTranslation::generateMove(int step)
{
    int timex[700] = {0};
    int accelx[700] = {0};
    int timey[700] = {0};
    int accely[700] = {0};
    // x
    int time = 1200;
    int dtime = 61035; // 25s
    int acce = 36;
    // y
    int accey = 5120;
    int atimey = 280;
    // back
    int backya = 280;
    int backyat = 2560;
    int backxa = 461;
    int backxat = 2415;

    int idx = 0;

    if(0 == step){
        timex[0] = time;
        timey[0] = time;
        accelx[0] = 0;
        accely[0] = acce;

        timex[1] = dtime;
        timey[1] = dtime;
        accelx[1] = 0;
        accely[1] = 0;

        timex[2] = time;
        timey[2] = time;
        accelx[2] = 0;
        accely[2] = -acce;

        idx = 3;
    }else if(1 == step){
        timex[0] = atimey;
        timey[0] = atimey;
        accelx[0] = accey;
        accely[0] = 0;

        timex[1] = atimey;
        timey[1] = atimey;
        accelx[1] = -accey;
        accely[1] = 0;

        idx = 2;
    }else if(2 == step){
        timex[0] = time;
        timey[0] = time;
        accelx[0] = 0;
        accely[0] = -acce;

        timex[1] = dtime;
        timey[1] = dtime;
        accelx[1] = 0;
        accely[1] = 0;

        timex[2] = time;
        timey[2] = time;
        accelx[2] = 0;
        accely[2] = acce;

        idx = 3;
    }else if(3 == step){
        timex[0] = backyat;
        timey[0] = backxat;
        accelx[0] = -backya;
        accely[0] = -backxa;

        timex[1] = backyat;
        timey[1] = backxat;
        accelx[1] = backya;
        accely[1] = backxa;

        idx = 2;
    }else{
        return;
    }

    m_vectorMove.PrepareArbitrarySpeedData(0, idx, timex, accelx, 0);
    m_vectorMove.PrepareArbitrarySpeedData(1, idx, timey, accely, 0);
    m_vectorMove.TriggerArbitrarySpeed(3);
}

int CCLargeTranslation::getStatus()
{
    return m_vectorMove.GetStatus(0);
}

void CCLargeTranslation::stop()
{
    m_vectorMove.Stop();
}
