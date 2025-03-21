#include "MainWidget.h"
#include "ui_MainWidget.h"

#include <QDateTime>
#include <mutex>

#include "running_frame/CCRunningCombineForm.h" // 运行界面
#include "oct_forms/src/octproz.h"              // 调试界面
#include "statistic_frame/CCStatisticFrame.h"   // 数据界面
#include "CCSettingDialog.h"                      // 参数设置界面
#include "infrastructure/CCSignalBridge.h"      // 激活注销
#include "running_frame/CCSwitchRoleDialog.h"     // 切换角色

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    // signal
    connect(ui->buttonMainFrame, &QPushButton::clicked, this, &MainWidget::slot_buttonMainFrame_clicked);
    connect(ui->buttonData, &QPushButton::clicked, this, &MainWidget::slot_buttonData_clicked);
    connect(ui->buttonDebug, &QPushButton::clicked, this, &MainWidget::slot_buttonDebug_clicked);
    connect(ui->buttonStatus, &QPushButton::clicked, this, &MainWidget::slot_buttonStatus_clicked);
    connect(ui->buttonSettings, &QPushButton::clicked, this, &MainWidget::slot_buttonSettings_clicked);
    connect(ui->buttonRole, &QPushButton::clicked, this, &MainWidget::slot_buttonRole_clicked);
    connect(CCSignalBridge::instance(), &CCSignalBridge::setMask, this, &MainWidget::setMask);
    // init mask widget
    m_maskWidget = new QWidget(this,Qt::FramelessWindowHint);
    m_maskWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    m_maskWidget->setObjectName("widget");// 设置对象句,相当于css里的id
    m_maskWidget->setStyleSheet("#widget {background-color:rgba(39, 42, 51, 0.8);}");// 设置id对应元素的背景色
    // 更新当前时间
    connect(&m_systemTime, &QTimer::timeout, this, &MainWidget::slot_timeUpdate);
    m_systemTime.start(1000);

    // 添加界面
    m_runningForm = new CCRunningCombineForm(ui->stackedWidget); // 运行界面
    ui->stackedWidget->insertWidget(0, m_runningForm);
    // 数据页面
    m_statisticForm = new CCStatisticFrame(ui->stackedWidget) ;   // 数据界面
    ui->stackedWidget->insertWidget(1, m_statisticForm);
    // 调试界面
    m_debugForm = new OCTproZ(ui->stackedWidget);
    ui->stackedWidget->insertWidget(2, m_debugForm);
    // 普通用户隐藏调试界面
//    ui->buttonDebug->setVisible(false);
//    m_debugForm->setStyleFormat(m_debugForm);

    this->setContentsMargins(0,0,0,0);
    ui->stackedWidget->setContentsMargins(0,0,0,0);
    ui->dataPage->setContentsMargins(0,0,0,0);

    // 切换到主界面
    slot_buttonMainFrame_clicked();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::slot_timeUpdate()
{
    // 日期格式
    static QString dateFormat = R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" /><style type="text/css"> p, li { white-space: pre-wrap; }</style></head>
<body>
<p style=" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style="font-size:18pt; font-weight:700; font-family: 'MiSans Semibold';">%1</span></p>
<p style=" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style="font-size:10pt; font-weight:500; font-family: 'DIN Alternate';">%2</span>
<span style="font-size:10pt; font-weight:500;"> %3</span></p>
</body></html>)";
    QLocale locale;
    locale.setDefault(QLocale::Chinese);
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString week = locale.toString(QDateTime::currentDateTime(), "dddd");
    ui->labelDateTime->setText(dateFormat.arg(time, date, week));
}

void MainWidget::changeFrame(FRAME frame)
{
    // 选中时的效果
    static QString choseStyle =
            "background-color: #3B5AFD;"
            "background-image: url(:/images/tool-button-background.svg); "
            "background-position: center;"
            "color:rgba(255,255,255,1); border-radius:8px; font-size:20px; font-weight:500;";
    // 未被选中的效果
    static QString unChoseStyle =
            "background-color:rgba(217, 217, 217, 0.1); color:rgb(228,231,249); border-radius:8px; font-size:20px; font-weight:500;";
    // 更新样式
    QString mFrameStyle = "://images/home%1.svg";
    QString dataStyle = "://images/data%1.svg";
    QString debugStlye = "://images/debug%1.svg";
    ui->buttonMainFrame->setIcon(QIcon(mFrameStyle.arg(frame == MAIN_FRAME?"-white":"")));
    ui->buttonData->setIcon(QIcon(dataStyle.arg(frame == DATA_FRAME?"-white":"")));
    ui->buttonDebug->setIcon(QIcon(debugStlye.arg(frame == DEBUG_FRAME?"-white":"")));

    ui->buttonMainFrame->setStyleSheet(frame==MAIN_FRAME?choseStyle:unChoseStyle);
    ui->buttonData->setStyleSheet(frame==DATA_FRAME?choseStyle:unChoseStyle);
    ui->buttonDebug->setStyleSheet(frame==DEBUG_FRAME?choseStyle:unChoseStyle);
}
// TODO 更新设备状态
void MainWidget::setDeviceStatus(DEVICE_STATUS status, QString info)
{

    static QString deviceStatus = R"(QPushButton#buttonStatus{
                                background: rgba(255, 255, 255, 0);
                                background-image: url(:/images/device-status.svg);
                                background-origin: content;
                                background-position: top;
                                padding-top: 10px;
                                background-repeat: none;
                                text-align: bottom;
                                padding-bottom: 20px;
                                font-size: 12px;
                                font-weight:500;
                                color: %1;}";)";
    switch (status) {
    case DEVICE_SUCCESS:
        deviceStatus = deviceStatus.arg("#00C54E");
        break;
    case DEVICE_WARNING:
        deviceStatus = deviceStatus.arg("#4EC500");
        break;
    case DEVICE_ERROR:
        deviceStatus = deviceStatus.arg("#FF0000");
        break;
    }
    ui->buttonStatus->setStyleSheet(deviceStatus);
}

void MainWidget::setMask(bool show)
{
    if(show){
        m_maskWidget->move(ui->stackedWidget->pos());
        m_maskWidget->resize(ui->stackedWidget->size());
        m_maskWidget->show();
    }else{
        m_maskWidget->close();
    }
}

void MainWidget::slot_buttonMainFrame_clicked()
{
    emit CCSignalBridge::instance()->deactivateDebugSystem();
    m_debugForm->setStyleFormat(ui->stackedWidget, true);
    changeFrame(MAIN_FRAME);
    this->update();
    ui->stackedWidget->setCurrentIndex(MAIN_FRAME);
}

void MainWidget::slot_buttonData_clicked()
{
    m_debugForm->setStyleFormat(ui->stackedWidget, true);
    changeFrame(DATA_FRAME);
    this->update();
    ui->stackedWidget->setCurrentIndex(DATA_FRAME);
}

void MainWidget::slot_buttonDebug_clicked()
{
    emit CCSignalBridge::instance()->deactivateRunningSystem();
    m_debugForm->updateSiderBar();
    m_debugForm->setStyleFormat(ui->stackedWidget);
    changeFrame(DEBUG_FRAME);
    this->update();
    ui->stackedWidget->setCurrentIndex(DEBUG_FRAME);
}
//TODO 点击设备状态按钮：弹出设备状态框
void MainWidget::slot_buttonStatus_clicked()
{
   static int status = 0;
//   setDeviceStatus((DEVICE_STATUS)(++status % 3), "");
}
// 点击设置按钮
void MainWidget::slot_buttonSettings_clicked()
{
    static CCSettingDialog settingform;
    settingform.setModal(true);
    settingform.show();
    setMask(true);
}
// 点击切换角色按钮
void MainWidget::slot_buttonRole_clicked()
{
    static CCSwitchRoleDialog form;
    static std::once_flag oc;
    std::call_once(oc, [&]{
        connect(&form, &CCSwitchRoleDialog::sendRole, [&](quint32 role){
            QString lbsytle = R"(QLabel#labelUser{
                            color: #BEC1D1;
                            font-size: 14px;
                            font-weight:500;
                            margin-left:15px;;
                            margin-top:2px;
                            background-color:rgba(0, 0, 0, 0);
                            background-image: url(:/images/user-%1.svg);
                            background-repeat:none;
                        })";
            if(0 == role){
                ui->buttonDebug->setVisible(false);
                ui->labelUser->setText(QString::fromLocal8Bit(R"(<html><head/><body><p align="right">普通用户</p></body></html>)"));
                ui->labelUser->setStyleSheet(lbsytle.arg("common"));
                if(2 == ui->stackedWidget->currentIndex()){
                    this->slot_buttonMainFrame_clicked();
                }
            }else{
                ui->buttonDebug->setVisible(true);
                ui->labelUser->setText(QString::fromLocal8Bit(R"(<html><head/><body><p align="right">管理员 </p></body></html>)"));
                ui->labelUser->setStyleSheet(lbsytle.arg("admin"));
            }
        });

    });

    form.setModal(true);
    form.show();
    setMask(true);
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    // Full Screen
    if(e->key() == Qt::Key_Escape){
        static bool key = true;
        if(key) this->showFullScreen();
        else this->showMaximized();
        key = !key;
    }else if(e->key() == Qt::Key_Space){
        return;
    }
    QWidget::keyPressEvent(e);
}

void MainWidget::closeEvent(QCloseEvent *event)
{

}
