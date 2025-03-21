#include "CCSettingDialog.h"
#include "ui_CCSettingDialog.h"

#include <QMouseEvent>
#include <QDebug>
#include <QVBoxLayout>

#include "common/CCPublicTitleBar.h"
#include "infrastructure/OCTGlobal.h"
#include "infrastructure/CCSignalBridge.h"
#include "oct_algorithm/oct_params/settings.h"
#include "oct_algorithm/oct_params/octalgorithmparameters.h"
// 参数名称
#define PROC "processing"
#define STREAM "streaming"
#define PROC_FLIP_BSCANS "flip_bscans"
#define PROC_BITSHIFT "bitshift"
#define PROC_MIN "min"
#define PROC_MAX "max"
#define PROC_LOG "log"
#define PROC_COEFF "coeff"
#define PROC_ADDEND "addend"
#define PROC_RESAMPLING "resampling"
#define PROC_RESAMPLING_INTERPOLATION "resampling_interpolation"
#define PROC_RESAMPLING_C0 "resampling_c0"
#define PROC_RESAMPLING_C1 "resampling_c1"
#define PROC_RESAMPLING_C2 "resampling_c2"
#define PROC_RESAMPLING_C3 "resampling_c3"
#define PROC_DISPERSION_COMPENSATION "dispersion_compensation"
#define PROC_DISPERSION_COMPENSATION_D0 "dispersion_compensation_d0"
#define PROC_DISPERSION_COMPENSATION_D1 "dispersion_compensation_d1"
#define PROC_DISPERSION_COMPENSATION_D2 "dispersion_compensation_d2"
#define PROC_DISPERSION_COMPENSATION_D3 "dispersion_compensation_d3"
#define PROC_WINDOWING "windowing"
#define PROC_WINDOWING_TYPE "window_type"
#define PROC_WINDOWING_FILL_FACTOR "window_fill_factor"
#define PROC_WINDOWING_CENTER_POSITION "window_center_position"
#define PROC_FIXED_PATTERN_REMOVAL "fixed_pattern_removal"
#define PROC_FIXED_PATTERN_REMOVAL_CONTINUOUSLY "fixed_pattern_removal_continuously"
#define PROC_FIXED_PATTERN_REMOVAL_BSCANS "fixed_pattern_removal_bscans"
#define PROC_SINUSOIDAL_SCAN_CORRECTION "sinusoidal_scan_correction"
#define STREAM_STREAMING "streaming_enabled"
#define STREAM_STREAMING_SKIP "streaming_skip"

CCSettingDialog::CCSettingDialog(QWidget *parent) :
    CCRadiusDialog(parent),
    ui(new Ui::CCSettingDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    initTitle();
    loadSettings();
}

CCSettingDialog::~CCSettingDialog()
{
    delete ui;
}

void CCSettingDialog::initTitle()
{
    QPalette pal(ui->widget->palette());
    //设置背景黑色
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Foreground, Qt::black);
    ui->widget->setAutoFillBackground(true);
    ui->widget->setPalette(pal);
    ui->widget->setTitle(u8"OCT运行参数");
}
// 显示时读取参数
void CCSettingDialog::showEvent(QShowEvent *event)
{
    AcquisitionParams sysParms = OCTGlobal::currentSystem->params->params;
//    ui->ldt_dataFile->setText(sysParms);
    ui->ldt_configFile->setText(sysParms.procFilePath);
    ui->spin_bit->setValue(sysParms.bitDepth);
    ui->spin_samplesPerAscan->setValue(sysParms.samplesPerAline);
    ui->spin_ascansPerBscan->setValue(sysParms.alinesPerBscan);
    ui->spin_bscansPerVolume->setValue(sysParms.buffersPerVolume * sysParms.bscansPerBuffer);
    ui->spin_buffersPerVolume->setValue(sysParms.buffersPerVolume);
//    ui->spin_buffersFromFile->setValue(sysParms.);
//    ui->spin_waitForLoadFile
    ui->spin_scanRangeX->setValue(sysParms.lengthX);
    ui->spin_scanRangeY->setValue(sysParms.lengthY);
    ui->dspin_maxScanRangeZ->setValue(sysParms.lengthZ);
    auto octParams = OctAlgorithmParameters::getInstanceForDebug();
    ui->spin_runningBufferNr->setValue(octParams->frameNrRunView);
    ui->spin_runningBufferNr->setMaximum(octParams->ascansPerBscan);
    QDialog::showEvent(event);
}

void CCSettingDialog::loadSettings()
{
    Settings* settingsManager = Settings::getInstance();

    //load setting maps
    this->processingSettings = settingsManager->getStoredSettings(PROC);
    this->streamingSettings = settingsManager->getStoredSettings(STREAM);


}

void CCSettingDialog::updateProcessingParams() {

}

void CCSettingDialog::updateStreamingParams() {

}

void CCSettingDialog::updateResamplingParams() {

}

void CCSettingDialog::updateDispersionParams() {

}

void CCSettingDialog::updateWindowingParams() {

}

void CCSettingDialog::saveSettings()
{

}

void CCSettingDialog::on_saveButton_clicked()
{
    OctAlgorithmParameters::getInstanceForDebug()->frameNrRunView = ui->spin_runningBufferNr->value();
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::accept();
}

void CCSettingDialog::on_cancelButton_clicked()
{
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::reject();
}
