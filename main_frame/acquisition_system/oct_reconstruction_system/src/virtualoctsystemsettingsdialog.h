/**
  * @author ld
  * @brief  recontruction system written to fit ZJU data and system
  * @date   2023-01-09
  */

#ifndef VIRTUALOCTSYSTEMDIALOG_H
#define VIRTUALOCTSYSTEMDIALOG_H

// parameters
#define SYSNAME "sys_name"
#define FILEPATH "data_path"
#define PARAPATH "para_path"
#define BITDEPTH "bit_depth"
#define WIDTH "width"
#define HEIGHT "height"
#define DEPTH "depth"
#define BUFFERS_PER_VOLUME "buffers_per_volume"
#define BUFFERS_FROM_FILE "buffers_from_file"
#define XRANGE "x_range"
#define YRANGE "y_range"
#define ZRANGE "z_range"
#define WAITTIME "wait_time"
#define COPY_TO_RAM "copy_file_to_ram"


#include <QStandardPaths>
#include <QVariant>
#include <QDialog>
#include <QString>
#include <QFileDialog>

#include "ui_virtualoctsystemsettingsdialog.h"

struct simulatorParams {
    QString filePath;       // data file
    QString paraPath;       // para file
    int bitDepth;           // bit depth
    int width;              // samples per ascans
    int height;             // ascans per bscan
    int depth;              // bscans per buffer
    int buffersPerVolume;   // buffers per volume
    int buffersFromFile;    // buffers from file
    int waitTimeUs;         // wait time us
    int xRange;             // scan x range
    int yRange;             // scan y range
    int maxZRange;          // scan max z range
    bool copyFileToRam;
};

class VirtualOCTSystemSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	VirtualOCTSystemSettingsDialog(QWidget *parent = nullptr);
	~VirtualOCTSystemSettingsDialog();
    // set settings from variantmap
	void setSettings(QVariantMap settings);
    // get settings from variantmap
	void getSettings(QVariantMap* settings);

private:
	Ui::VirtualOCTSystemSettingsDialog* ui;
	simulatorParams params;
    // init gui
	void initGui();

public slots:
    // select data file
	void slot_selectFile();
    // select para file
    void slot_selectParaFile();
    // applay clicked
	void slot_apply();
    // disable gui in running mode in case changing param
	void slot_enableGui(bool enable);
    // check if width can divide by 2
	void slot_checkWidthValue();

signals:
    // update settings
	void settingsUpdated(simulatorParams newParams);
};

#endif // VIRTUALOCTSYSTEMDIALOG_H
