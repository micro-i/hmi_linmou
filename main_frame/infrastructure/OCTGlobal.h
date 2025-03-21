#ifndef OCTGLOBAL_H
#define OCTGLOBAL_H
#include <QString>
#include <QMutex>
#include <QThread>
#include "oct_devkit/src/acquisitionsystem.h"

// 设置相关宏
#define SETTINGS_DIR QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
#define SETTINGS_FILE_NAME "settings.ini"
#define SETTINGS_PATH  SETTINGS_DIR + "/" + SETTINGS_FILE_NAME
#define TIMESTAMP "timestamp"

namespace OCTGlobal {
// 当前采集系统
extern AcquisitionSystem* currentSystem;

extern QString systemType;
// 当前采集系统运行
extern bool realtimeAcquisition;
// 是否为运行模式
extern bool isInRunningMode;
//****** 此锁非常重要！！！！！初始化GL以及注册GLbuf资源时使用
extern QMutex glContextMtx;
// log path
extern QString m_logPath;
//
extern QString globalStyle;
//
extern QString lastSelectedFolder;
}

#endif // OCTGLOBAL_H
