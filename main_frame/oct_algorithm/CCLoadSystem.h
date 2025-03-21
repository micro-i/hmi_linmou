/**
  * @author ld
  * @brief 本类用于从文件中读取dll作为采集系统
  * @date    2022-11-10
  */

#ifndef CCLOADSYSTEM_H
#define CCLOADSYSTEM_H

#include <QObject>

class CCLoadSystem : public QObject
{
    Q_OBJECT
public:
    /** 单例
     * @brief instance
     * @return
     */
    static CCLoadSystem& instance();
    /**
     * @brief loadSystem 采集系统读取器
     */
    QT_DEPRECATED void loadSystem();
signals:
    void info(QString info);
    void error(QString error);
private:
    CCLoadSystem();
    QString m_systemPath;
};

#endif // CCLOADSYSTEM_H
