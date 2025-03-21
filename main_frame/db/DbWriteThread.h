#ifndef DBWRITETHREAD_H
#define DBWRITETHREAD_H

#include <QMutex>
#include <QQueue>
#include <QSqlDatabase>
#include <QThread>
#include <QWaitCondition>

//! 数据库写入线程
class DbWriteThread : public QThread
{
    Q_OBJECT

public:

    //!  数据库写入任务
    struct Task {
        QString   sql;
        quint64   id;
        Task(const QString& tsql, quint64 tid): sql(tsql), id(tid) {}
        Task(const Task &other) : sql(other.sql), id(other.id) {}
        Task() = default;
    };
    /**
     * @brief DbWriteThread 构造
     * @param id id
     * @param db 数据库
     */
    explicit DbWriteThread(uint64_t id, QSqlDatabase db = QSqlDatabase());
    /**
     * @brief ~DbWriteThread 析构
     */
    virtual ~DbWriteThread() override;
    /**
     * @brief addTask 添加执行语句
     * @param sql 执行的数据库语句
     * @param id id
     */
    void addTask(const QString &sql, quint64 id = 0);
    /**
     * @brief 停止线程
     */
    void stopThread();
signals:
    /**
     * @brief writeBufferSizeUpdated 任务队列大小更新
     * @param size 大小
     */
    void writeBufferSizeUpdated(int size);
protected:
    void run() override;
    QQueue<Task>    m_taskBuffer;       // 任务队列
    bool            m_enabled = true;   // 是否启用
    uint64_t        m_id;               // id
    QMutex          m_mutex;            // 数据处理锁
    QWaitCondition  m_hasTask;          // 等待处理条件
    QSqlDatabase    m_db;               // 数据库
};

#endif // DBWRITETHREAD_H
