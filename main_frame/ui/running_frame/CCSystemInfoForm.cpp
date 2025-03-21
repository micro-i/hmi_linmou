#include "CCSystemInfoForm.h"
#include "ui_CCSystemInfoForm.h"

#include <QGraphicsDropShadowEffect>            // for box shadow

#include "infrastructure/CCSignalBridge.h"      // for update params;
#include "infrastructure/OCTGlobal.h"           // get current system
#include "oct_algorithm/oct_params/settings.h"  // read settings
#include "CCChoseProductDialog.h"               // chose product
#include "db/DbClientBase.h"
#include "CCProductManageDialog.h"

#define BLUR_RADIUS 36

CCSystemInfoForm::CCSystemInfoForm(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SystemInfoForm)
{
    ui->setupUi(this);
    // system relate
    connect(CCSignalBridge::instance(), &CCSignalBridge::deactivateRunningSystem, this, &CCSystemInfoForm::on_buttonStop_clicked);
    // Set Box Shadow;
    auto getShadow = [](QColor color, int radius, QWidget* tparent = nullptr){
        auto shadowEffect = new QGraphicsDropShadowEffect(tparent);
        shadowEffect->setOffset(0, 0);
        shadowEffect->setColor(color);
        shadowEffect->setBlurRadius(radius);
        return shadowEffect;
    };
    ui->buttonPlay->setGraphicsEffect(getShadow(QColor(59, 90, 253, 76), BLUR_RADIUS + 2, ui->buttonPlay));
    ui->buttonStop->setGraphicsEffect(getShadow(QColor(59, 90, 253, 76), BLUR_RADIUS, ui->buttonPlay));
    ui->buttonProduct->setGraphicsEffect(getShadow(QColor(59, 90, 253, 76), BLUR_RADIUS, ui->buttonPlay));
    activeCurrentSystem();
}

CCSystemInfoForm::~CCSystemInfoForm()
{
    if(OCTGlobal::currentSystem)
        deactivateSystem(OCTGlobal::currentSystem);
    delete ui;
    qDebug() << "CCSystemInfoForm destructor. Thread ID: " << QThread::currentThreadId();
}

void CCSystemInfoForm::updateProgress(const int cur, const int total)
{
    // 更新进度条
    ui->progressBar->setValue(cur);
    ui->progressBar->setMaximum(total);
    // 更新目标块
    QString style =QString::fromLocal8Bit(R"(<html><head/><body><p>
                                          <span style="font-size:16px; font-weight:400;">正在运行：第</span>
                                          <span style="color:#3B5AFD; font-size:17px; font-weight:500;">%1</span>
                                          <span style="font-size:16px; font-weight:400;">块，共</span>
                                          <span sytle="color:#1B1B24; font-size:17px; font-weight:500;">%2</span>
                                          <span style="font-size:16px; font-weight:400;">块</span></p></body></html>)");
    ui->labelProgressText->setText(style.arg(QString::number(cur), QString::number(total)));
}

void CCSystemInfoForm::setCurrentConfigName(const QString& name)
{
    ui->labelPlanText->setText(name);
}

void CCSystemInfoForm::enableStopButton()
{
    ui->labelStop->setStyleSheet(R"(QLabel{
                                 background-color:rgba(0, 0, 0, 0);
                                 color: #101820;
                                 font-size: 16px;
                                 font-weight: 500;
                                 })");
    ui->buttonStop->setEnabled(true);
}

void CCSystemInfoForm::on_buttonPlay_clicked()
{
    if(OCTGlobal::currentSystem && OCTGlobal::currentSystem->acquisitionRunning){
        qWarning() << "Acquisition is Running";
        return;
    }else if(!OCTGlobal::currentSystem){
        qWarning() << "System do not exist";
        return;
    }

    CCChoseProductDialog form(nullptr);
    emit CCSignalBridge::instance()->setMask(true);
    if(QDialog::Accepted != form.exec()){
        qDebug() << "Start Canceled";
        emit CCSignalBridge::instance()->setMask(false);
        return;
    }
    emit CCSignalBridge::instance()->setMask(false);

    quint32 lTaskID = DbClientBase::instance()->getLastTaskID();
    // 设置当前配置名称
    this->setCurrentConfigName(form.m_product.productName);
    // 设置当前产品批次号
    ui->labelProductIDText->setText("NO." + QString("%1").arg(lTaskID + 1, 6, 10, QChar('0')));
    emit init(lTaskID + 1, form.m_product.productId, form.m_batchNo);
    // 显示为正在启动中
    ui->labelProgressText->setText(QString(u8"正在启动采集系统中..."));
    emit start();
    qApp->processEvents();
    ui->labelPlay->setStyleSheet(R"(QLabel{
                                     background-color:rgba(0, 0, 0, 0);
                                     color: #101820;
                                     font-size: 16px;
                                     font-weight: 500;
                                     })");
    ui->labelProduct->setStyleSheet("QLabel{\n"
                                    "background-color:rgba(0, 0, 0, 0);\n"
                                    "color: rgba(0, 0, 0, 0.3);\n"
                                    "font-size: 16px;\n"
                                    "font-weight: 400;\n"
                                    "}");
    ui->buttonPlay->setEnabled(false);
    ui->buttonProduct->setEnabled(false);
}

void CCSystemInfoForm::on_buttonStop_clicked()
{
    ui->buttonStop->setEnabled(false);
    if (OCTGlobal::currentSystem != nullptr) {
        if(OCTGlobal::currentSystem->acquisitionRunning){
            OCTGlobal::currentSystem->stopAcquisition();
        }
    }
    ui->buttonPlay->setEnabled(true);
    ui->buttonProduct->setEnabled(true);
    QString style =QString::fromLocal8Bit(R"(<html><head/><body><p>
                                          <span style="font-size:16px; font-weight:400;">等待启动：第</span>
                                          <span style="color:#3B5AFD;font-size:17px; font-weight:500;">0</span>
                                          <span style="font-size:16px; font-weight:400;">块，共</span>
                                          <span sytle="color:#1B1B24;font-size:17px; font-weight:500;">0</span>
                                          <span style="font-size:16px; font-weight:400;">块</span></p></body></html>)");
    updateProgress(0, 5);
    // 更新目标块
    ui->labelProgressText->setText(style);
    ui->labelPlay->setStyleSheet(R"(QLabel{
                                     background-color:rgba(0, 0, 0, 0);
                                     color: #101820;
                                     font-size: 16px;
                                     font-weight: 400;
                                     })");
    ui->labelStop->setStyleSheet(R"(QLabel{
                                 background-color:rgba(0, 0, 0, 0);
                                 color: rgba(0, 0, 0, 0.3);
                                 font-size: 16px;
                                 font-weight: 400;
                                 })");
    ui->labelProduct->setStyleSheet("QLabel{\n"
                                    "background-color:rgba(0, 0, 0, 0);\n"
                                    "color: #101820;\n"
                                    "font-size: 16px;\n"
                                    "font-weight: 400;\n"
                                    "}");
}

void CCSystemInfoForm::on_buttonProduct_clicked()
{
    // TODO 创建产品
    static CCProductManageDialog dialog;
    emit CCSignalBridge::instance()->setMask(true);
    dialog.setModal(true);
    dialog.show();
}

void CCSystemInfoForm::activateSystem(AcquisitionSystem* system) {
    if(system != nullptr){
        system->moveToThread(&acquisitionThread) ;
        connect(this, &CCSystemInfoForm::start, system, &AcquisitionSystem::startAcquisition, Qt::UniqueConnection);
        connect(system, &AcquisitionSystem::acquisitionStopped, this, &CCSystemInfoForm::on_buttonStop_clicked, Qt::UniqueConnection);
        // 读取系统设置
        system->settingsLoaded(Settings::getInstance()->getStoredSettings(system->getName()));
        // 更改设置时，保存设置
        connect(system, &AcquisitionSystem::storeSettings,
                CCSignalBridge::instance(), &CCSignalBridge::slot_storePluginSettings, Qt::UniqueConnection);
        // 更新参数
        connect(system->params, &AcquisitionParameter::updated,
                CCSignalBridge::instance(), &CCSignalBridge::slot_updateAcquistionParameter, Qt::UniqueConnection);
        // 信息输出
        connect(system, &AcquisitionSystem::info, [](QString msg){
            qDebug() << msg;
        });
        connect(system, &AcquisitionSystem::error, [](QString msg){
            qWarning() << msg;
        });
        acquisitionThread.start();
    }
}

void CCSystemInfoForm::deactivateSystem(AcquisitionSystem* system) {
    on_buttonStop_clicked();
    disconnect(this, &CCSystemInfoForm::start,
            system, &AcquisitionSystem::startAcquisition);
    system->disconnect();
    disconnect(system->params, &AcquisitionParameter::updated,
               nullptr, nullptr);
    disconnect(this, &CCSystemInfoForm::start, nullptr, nullptr);
    acquisitionThread.quit();
    acquisitionThread.wait();
    qDebug() << "System Deactivated";
}

void CCSystemInfoForm::activeCurrentSystem()
{
    if(OCTGlobal::currentSystem)
    activateSystem(OCTGlobal::currentSystem);
}

void CCSystemInfoForm::deactivateCurrentSystem()
{
    if(OCTGlobal::currentSystem)
    deactivateSystem(OCTGlobal::currentSystem);
}
