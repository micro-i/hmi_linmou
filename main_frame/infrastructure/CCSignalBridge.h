#ifndef CCSIGNALBRIDGE_H
#define CCSIGNALBRIDGE_H
#include <QObject>

#include <oct_devkit/src/acquisitionparameter.h>

//! 转接信号类
class CCSignalBridge : public QObject
{
    Q_OBJECT
public:
    static CCSignalBridge* instance();
signals:
    /**
     * @brief 注销运行系统
     */
    void deactivateRunningSystem();
    /**
     * @brief 激活运行系统
     */
    void activateRunningSystem();
    /**
     * @brief 注销调试系统
     */
    void deactivateDebugSystem();
    /**
     * @brief 激活调试系统
     */
    void activateDebugSystem();
    /**
     * @brief setMask 设置遮罩是否显示
     * @param show
     */
    void setMask(bool show);
public:
    /**
     * @brief slot_storePluginSettings 采集系统保存设置
     * @param pluginName 插件名称
     * @param settings 设置
     */
    void slot_storePluginSettings(QString pluginName, QVariantMap settings);
    /**
     * @brief slot_updateAcquistionParameter 采集系统参数更新
     * @param newParams 采集参数
     */
    void slot_updateAcquistionParameter(AcquisitionParams newParams);
private:
    explicit CCSignalBridge(QObject *parent = nullptr);
};

#endif // CCSIGNALBRIDGE_H
