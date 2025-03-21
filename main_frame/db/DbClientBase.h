#ifndef DBCLIENTBASE_H
#define DBCLIENTBASE_H

// 高端质检迁移版本用于OCT

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include "DbModel.h"
#include "DbWriteThread.h"

//! 数据库处理类
class DbClientBase : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief instance 获取单例
     * @return
     */
    static DbClientBase* instance();
    /**
     * @brief DbClientBase 构造函数--备用
     * @param host
     * @param username
     * @param password
     * @param port
     * @param name
     * @param parent
     */
    explicit DbClientBase(const QString& host, const QString& username, const QString& password,
                          const int port, const QString& name, QObject *parent = nullptr);
    /**
     * @brief ~DbClientBase 析构
     */
    virtual ~DbClientBase();
    /**
     * @brief setName 设置数本类实例名称
     * @param name 实例名称
     */
    void setName(const QString &name);
    /**
     * @brief isValid
     * @return 是否可用
     */
    bool isValid();
    /**
     * @brief sqlDatabase 获取数据库副本
     * @return 数据库副本
     */
    QSqlDatabase sqlDatabase() const;
    /*-----------------------------------获取数据---------------------------------------*/
    /**
     * @brief 获取所有产品
     * @return 产品集合
     */
    QList<Db::Product> getProducts();
    /**
     * @brief 获取上一次的任务号
     * @return 上一次任务号
     */
    quint32 getLastTaskID();
    /**
     * @brief getUserConfig 获取配置值
     * @return 返回设置值
     */
    QString getUserConfig(QString configName);
    /*-----------------------------------存入数据---------------------------------------*/
    /**
     * @brief writeTask 写入任务
     * @param task 任务
     */
    void writeTask(const Db::Task& task);
    /**
     * @brief writeProductModelInspectResult 写入块结果
     * @param result 块结果
     */
    void writeProductModelInspectResult(const Db::ProductModelInspectResult& result);
    /**
     * @brief writeProductClassfiedResult 写入产品结果
     * @param result 块结果
     */
    void writeProductClassfiedResult(const Db::ProductClassfiedResult& result);
    /**
     * @brief 设置用户参数
     * @param configName 配置名称
     * @param value 配置值
     */
    void setUserConfig(QString configName, QString value);
    /*------------------------------------*/

signals:
    // 写入线程数据库语句数量变更
    void writeBufferSizeUpdated(uint64_t id, const QString &name, int size);

private:
    // 默认数据库
    DbClientBase(QObject *parent = nullptr);
    /**
     * @brief initDb 初始化数据库
     * @param host host
     * @param username 用户名
     * @param password 密码
     * @param port 端口
     * @param dbname 数据库名称
     */
    void initDb(const QString& host, const QString& username, const QString& password,
                const int port, const QString& dbname);
    /**
     * @brief checkAndCreateTable 检测表是否存在，如果不存在则构造
     */
    void checkAndCreateTable();
protected:
    void onWriteBufferSizeUpdated(int size);

    QSqlDatabase                        m_db;               // 数据库
    QString                             m_dbName;           // 数据库名称
    std::unique_ptr<DbWriteThread>      m_dbWriteThread;    // 数据库操作线程
    QString                             m_name;             // 本类实例名称
    quint64                             m_id;               // id
    bool                                m_isValid = false;  // 数据库是否可用
    static quint64                      m_counter;          // 本类每初始化一次数据库，连接数加一
    static QMutex                       m_mutex;            // 连接数锁
};

#endif // DBCLIENTBASE_H
