/**
  * @author ld
  * @brief  用于将读取的采集系统收集并保存
  *         待使用时从管理器中提取即可
  * @date    2022-11-10
  */


#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <QObject>
#include <QMap>

#include "oct_devkit/src/acquisitionsystem.h" // for acquisitionsystem

class CCSystemManager : public QObject
{
	Q_OBJECT
public:
    /**
     * @brief SystemManager 构造
     * @param parent
     */
    explicit CCSystemManager(QObject *parent = nullptr);
    /**
     * @brief ~CCSystemManager 析构
     */
    ~CCSystemManager();
    /**
     * @brief instance 运行系统管理器单例
     * @return
     */
    static CCSystemManager * instance();
    /**
     * @brief addSystem 添加系统
     * @param system
     */
    void addSystem(AcquisitionSystem* system, bool isDebug = false);
    /**
     * @brief getSystemByName 根据名称返回系统
     * @param name
     * @return
     */
    AcquisitionSystem* getSystemByName(QString name, bool isDebug = false);
    /**
     * @brief getSystems 获取全部系统
     * @return
     */
    QList<AcquisitionSystem*> getSystems(bool isDebug = false);
    /**
     * @brief getSystemNames 获取全部系统名称（目前名称不可重复）
     * @return
     */
    QList<QString> getSystemNames(bool isDebug = false);

private:
    QMap<QString, AcquisitionSystem*> m_systems;  // <系统名称, 采集系统指针>
    QMap<QString, AcquisitionSystem*> m_debugSystems;  // <系统名称, 采集系统指针>
};

#endif // SYSTEMMANAGER_H
