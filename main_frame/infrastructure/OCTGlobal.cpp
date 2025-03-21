#include "OCTGlobal.h"

namespace OCTGlobal {
/*---------采集系统---------*/
AcquisitionSystem* currentSystem = nullptr;
bool realtimeAcquisition = true;
bool isInRunningMode = false;
QString systemType = "debug";

QMutex glContextMtx;
QString globalStyle;

#ifdef Q_OS_LINUX
QString m_logPath = "/home/adt/data/hmi_oct/logs";
#else
QString m_logPath = "./data/logs";
#endif

QString lastSelectedFolder = "";
}
