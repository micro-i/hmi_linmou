#include "CCSignalBridge.h"

#include "oct_algorithm/oct_params/settings.h"
#include "oct_algorithm/oct_params/octalgorithmparameters.h"

CCSignalBridge* CCSignalBridge::instance()
{
    // INFO CCSignalBridge静态类
    static CCSignalBridge bridge(nullptr);
    return &bridge;
}


CCSignalBridge::CCSignalBridge(QObject *parent) : QObject(parent)
{

}

void CCSignalBridge::slot_storePluginSettings(QString pluginName, QVariantMap settings) {
    Settings::getInstance()->storeSettings(pluginName, settings);
}

void CCSignalBridge::slot_updateAcquistionParameter(AcquisitionParams newParams){
    auto octParams = OctAlgorithmParameters::getInstanceForDebug();
    octParams->samplesPerLine = newParams.samplesPerAline;
    octParams->ascansPerBscan = newParams.alinesPerBscan;
    octParams->bscansPerBuffer = newParams.bscansPerBuffer;
    octParams->buffersPerVolume = newParams.buffersPerVolume;
    octParams->bitDepth = newParams.bitDepth;
    octParams->acquisitionParamsChanged = true;
}
