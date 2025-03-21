#include "virtualoctsystemsettingsdialog.h"
#include <QStyleFactory>

VirtualOCTSystemSettingsDialog::VirtualOCTSystemSettingsDialog(QWidget *parent)
	: ui(new Ui::VirtualOCTSystemSettingsDialog) //QDialog(parent)
{
    qRegisterMetaType<simulatorParams >("simulatorParams");
    // 去掉问号，只保留关闭
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    ui->setupUi(this);
	initGui();
}

VirtualOCTSystemSettingsDialog::~VirtualOCTSystemSettingsDialog()
{
}

void VirtualOCTSystemSettingsDialog::setSettings(QVariantMap settings){
    this->ui->ldt_data_file->setText(settings.value(FILEPATH).toString());
    this->ui->ldt_parameter_file->setText(settings.value(PARAPATH).toString());
	this->ui->spinBox_bitDepth->setValue(settings.value(BITDEPTH).toInt());
    this->ui->spinBox_samplesPerAscan->setValue(settings.value(WIDTH).toInt());
    this->ui->spinBox_ascansPerBscan->setValue(settings.value(HEIGHT).toInt());
    this->ui->spinBox_bscansPerVolume->setValue(settings.value(DEPTH).toInt());
	this->ui->spinBox_buffersPerVolume->setValue(settings.value(BUFFERS_PER_VOLUME).toInt());
	this->ui->spinBox_buffersFromFile->setValue(settings.value(BUFFERS_FROM_FILE).toInt());
	this->ui->spinBox_waitTime->setValue(settings.value(WAITTIME).toInt());
    this->ui->spinBox_xRange->setValue(settings.value(XRANGE).toInt());
    this->ui->spinBox_yRange->setValue(settings.value(YRANGE).toInt());
    this->ui->doubleSpinBox->setValue(settings.value(ZRANGE).toDouble());
	this->ui->checkBox_copyFileToRam->setChecked(settings.value(COPY_TO_RAM).toBool());
	this->slot_apply();
}

void VirtualOCTSystemSettingsDialog::getSettings(QVariantMap* settings) {
    settings->insert(FILEPATH, this->ui->ldt_data_file->text());
    settings->insert(PARAPATH, this->ui->ldt_parameter_file->text());
	settings->insert(BITDEPTH, this->ui->spinBox_bitDepth->value());
    settings->insert(WIDTH, this->ui->spinBox_samplesPerAscan->value());
    settings->insert(HEIGHT, this->ui->spinBox_ascansPerBscan->value());
    settings->insert(DEPTH, this->ui->spinBox_bscansPerVolume->value());
	settings->insert(BUFFERS_PER_VOLUME, this->ui->spinBox_buffersPerVolume->value());
	settings->insert(BUFFERS_FROM_FILE, this->ui->spinBox_buffersFromFile->value());
	settings->insert(WAITTIME, this->ui->spinBox_waitTime->value());
    settings->insert(XRANGE, this->ui->spinBox_xRange->value());
    settings->insert(YRANGE, this->ui->spinBox_yRange->value());
    settings->insert(ZRANGE, this->ui->doubleSpinBox->value());
	settings->insert(COPY_TO_RAM, this->ui->checkBox_copyFileToRam->isChecked());
}

void VirtualOCTSystemSettingsDialog::initGui(){
    this->setWindowTitle(tr("Reconstruction OCT System Settings"));
    connect(this->ui->pb_selectDataFile, &QPushButton::clicked, this, &VirtualOCTSystemSettingsDialog::slot_selectFile);
    connect(this->ui->pb_selectParameterFile, &QPushButton::clicked, this, &VirtualOCTSystemSettingsDialog::slot_selectParaFile);
	connect(this->ui->okButton, &QPushButton::clicked, this, &VirtualOCTSystemSettingsDialog::slot_apply);
    connect(this->ui->spinBox_samplesPerAscan, &QSpinBox::editingFinished, this, &VirtualOCTSystemSettingsDialog::slot_checkWidthValue);
}

void VirtualOCTSystemSettingsDialog::slot_selectFile(){
    QString currentPath = this->ui->ldt_data_file->text();
	QString standardLocation = this->params.filePath.size() == 0 ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : this->params.filePath;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raw OCT Volume "), standardLocation, tr("Raw OCT Volume File (*.raw)"));
	if (fileName == "") {
		fileName = currentPath;
	}
    this->ui->ldt_data_file->setText(fileName);
}

void VirtualOCTSystemSettingsDialog::slot_selectParaFile(){
    QString currentPath = this->ui->ldt_parameter_file->text();
    QString standardLocation = this->params.paraPath.size() == 0 ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : this->params.paraPath;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open OCT Parameter File"), standardLocation, tr("OCT Parameter File (*.ini)"));
    if (fileName == "") {
        fileName = currentPath;
    }
    this->ui->ldt_parameter_file->setText(fileName);
}

void VirtualOCTSystemSettingsDialog::slot_apply() {
    this->params.filePath = this->ui->ldt_data_file->text();
    this->params.paraPath = this->ui->ldt_parameter_file->text();
	this->params.bitDepth = this->ui->spinBox_bitDepth->value();
    this->params.width = this->ui->spinBox_samplesPerAscan->value();
    this->params.height = this->ui->spinBox_ascansPerBscan->value();
    this->params.depth = this->ui->spinBox_bscansPerVolume->value() / ui->spinBox_buffersPerVolume->value();
	this->params.buffersPerVolume = this->ui->spinBox_buffersPerVolume->value();
	this->params.buffersFromFile = this->ui->spinBox_buffersFromFile->value();
	this->params.waitTimeUs = this->ui->spinBox_waitTime->value();
    this->params.xRange = this->ui->spinBox_xRange->value();
    this->params.yRange = this->ui->spinBox_yRange->value();
    this->params.maxZRange = this->ui->doubleSpinBox->value();
	this->params.copyFileToRam = this->ui->checkBox_copyFileToRam->isChecked();
	emit settingsUpdated(this->params);
}

void VirtualOCTSystemSettingsDialog::slot_enableGui(bool enable){
    this->ui->ldt_data_file->setEnabled(enable);
    this->ui->pb_selectDataFile->setEnabled(enable);
    this->ui->pb_selectParameterFile->setEnabled(enable);
	this->ui->spinBox_bitDepth->setEnabled(enable);
    this->ui->spinBox_samplesPerAscan->setEnabled(enable);
    this->ui->spinBox_ascansPerBscan->setEnabled(enable);
    this->ui->spinBox_bscansPerVolume->setEnabled(enable);
	this->ui->spinBox_buffersPerVolume->setEnabled(enable);
	this->ui->spinBox_buffersFromFile->setEnabled(enable);
	//this->ui->spinBox_waitTime->setEnabled(enable);  //waitTime does not need to be disabled. It can be safely changed during processing
	this->ui->checkBox_copyFileToRam->setEnabled(enable);
}

void VirtualOCTSystemSettingsDialog::slot_checkWidthValue(){
    int width = this->ui->spinBox_samplesPerAscan->value();
	if(width % 2 != 0){
        this->ui->spinBox_samplesPerAscan->setValue(width-1);
	}
}
