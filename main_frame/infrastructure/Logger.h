#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class CCLogSendServer;

class Logger : public QThread
{
    Q_OBJECT
public:
    virtual ~Logger() override;
    /**
     * @brief 获取单例Logger类
     * @return
     */
    static Logger *instance();
    /**
     * @brief QtMessageWrapper
     */
    static void messageWrapper(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    /**
     * @brief 设置保存路径
     * @param path 路径
     */
    static void setLogSavePath(const QString &path);

signals:
    /* 当产生的日志时，此信号被发送. 建议UI的日志界面应该绑定此信号 */
    void newMessage(QtMsgType type, QString fileName, QString text);

    /* 当产生的日志类型为Critical时，此信号被发送. 建议UI主界面绑定此信号,当严重错误发生时，弹框告知用户。 */
    void critical(QString text);

protected:
    virtual void run() override;

private:
    Logger(QObject *parent = nullptr);                  // 构造
    void add(QString text);                           // 添加消息
    static QString msgTypeToString(QtMsgType type);   // 消息类型转字符串
    bool m_enable = true;                               // 结束时推出线程标志
    QStringList m_waitBuffer;                           // 消息队列
    QMutex m_mutex;
    QWaitCondition m_hasTask;                           // 是否有未处理消息
};

#endif // LOGGER_H
