#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QMutex>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <vectormove.h>

QMutex mtx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int ret = vectorMove::SetComPortn(4);
//    connect(&m_timer, &QTimer::timeout, [&](){
//        {
//            QMutexLocker lck(&mtx);
//            ui->ldt_xspeed->setText(vectorMove::GetDisplay(0));
//            ui->ldt_yspeed->setText(vectorMove::GetDisplay(1));
//        }
//        qApp->processEvents();
//    });
    m_timer.start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
    vectorMove::CloseComPort();
}


void MainWindow::on_pushButton_clicked()
{
    QMutexLocker lck(&mtx);
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
    int atimey = 320;
    // back
    int backya = 320;
    int backyat = 2560;
    int backxa = 461;
    int backxat = 2415;

    //line 1
    for(int i = 0; i<24; ++i){
        if(0 == i || 10 == i || 20 == i){
            timex[i] = time;
            timey[i] = time;
            accelx[i] = 0;
            accely[i] = acce;

            timex[i+1] = dtime;
            timey[i+1] = dtime;
            accelx[i+1] = 0;
            accely[i+1] = 0;

            timex[i+2] = time;
            timey[i+2] = time;
            accelx[i+2] = 0;
            accely[i+2] = -acce;
        }else if(3 == i || 8 == i || 13 == i || 18 == i){
            timex[i] = atimey;
            timey[i] = atimey;
            accelx[i] = accey;
            accely[i] = 0;

            timex[i+1] = atimey;
            timey[i+1] = atimey;
            accelx[i+1] = -accey;
            accely[i+1] = 0;
        }else if(5 == i || 15 == i){
            timex[i] = time;
            timey[i] = time;
            accelx[i] = 0;
            accely[i] = -acce;

            timex[i+1] = dtime;
            timey[i+1] = dtime;
            accelx[i+1] = 0;
            accely[i+1] = 0;

            timex[i+2] = time;
            timey[i+2] = time;
            accelx[i+2] = 0;
            accely[i+2] = acce;
        }else if(23 == i){
            timex[i] = backyat;
            timey[i] = backxat;
            accelx[i] = -backya;
            accely[i] = -backxa;

            timex[i+1] = backyat;
            timey[i+1] = backxat;
            accelx[i+1] = backya;
            accely[i+1] = backxa;
        }
    }




    //    vectorMove::SetMaxSpeed(ui->cb_direction->currentIndex(), ui->spin_maxspeed->value());
//    vectorMove::SetDistance(ui->cb_direction->currentIndex(), ui->spin_distance->value());
    //    vectorMove::SetSpeed(ui->cb_direction->currentIndex(), ui->spin_speed->value());
    vectorMove::PrepareArbitrarySpeedData(0, 25, timex, accelx, 0);
    vectorMove::PrepareArbitrarySpeedData(1, 25, timey, accely, 0);

    vectorMove::TriggerArbitrarySpeed(3);

//    vectorMove::SetDistance(1, 12800);
}

void MainWindow::on_pushButton_2_clicked()
{
    vectorMove::Stop();
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->ldt_yspeed->setText(QString::number(vectorMove::GetStatus(0)));
}
