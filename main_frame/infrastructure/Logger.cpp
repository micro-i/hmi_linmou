#include "Logger.h"

#include <QDateTime>
#include <QDir>
#include <QCoreApplication>

#include <memory>

#include "infrastructure/OCTGlobal.h"
using OCTGlobal::m_logPath;

Logger::Logger(QObject *parent)
    : QThread(parent)
{
    // 创建日志保存路径
    QDir dir;
    if (!dir.exists(m_logPath)) {
        dir.mkpath(m_logPath);
    }
    // 开始监控
    start();
}

Logger::~Logger()
{
    m_enable = false;
    m_hasTask.wakeAll();
    this->quit();
    this->wait();
    printf("Logger destroyd\n");
}

Logger *Logger::instance()
{
    static Logger logger;
    return &logger;
}

void Logger::messageWrapper(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 消息文件
    QString fn = QString(context.file);
#ifdef Q_OS_LINUX
    QString msgFile = fn.mid(fn.lastIndexOf("/") + 1);
#else
    QString msgFile = fn.mid(fn.lastIndexOf("\\") + 1);
#endif
    // 线程ID
    qint64 threadId = reinterpret_cast<qint64>(QThread::currentThreadId());
    // 消息
    QString log = QString("[%1 %2 %3][%4:%5]: | %6")
            .arg(msgTypeToString(type),
                 QDateTime::currentDateTime().time().toString("hh:mm:ss:zzz"),
                 QString::number(threadId, 16).toUpper(),
                 msgFile)                                                       // 消息文件
            .arg(context.line)                                                  // 消息行数
            .arg(QString(msg.toLocal8Bit()));                                   // 消息
    // 打印消息
    printf("%s\n", log.toUtf8().data());
    QCoreApplication::processEvents();
    //  警告信号
    emit instance()->newMessage(type, msgFile, log);
    switch (type) {
    case QtCriticalMsg:
        emit instance()->critical(msg);
        break;
    default:
        break;
    }
    // 写出文件
    instance()->add(log);
}

void Logger::setLogSavePath(const QString &path)
{
    m_logPath = path;

    QDir dir;
    if (!dir.exists(m_logPath)) {
        dir.mkpath(m_logPath);
    }
}

void Logger::run()
{
    // 写出方法
    auto callWrite = [](const QString& text){
        QFile f(m_logPath + QString("/%1.log")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd-HH")));
        if (f.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            f.write(text.toLocal8Bit());
            f.write("\n");
            f.close();
        } else {
            printf("ERORRRRRRRRR write log faild, log file can not open\n");
        }
    };

    while (m_enable) {
        QString text;
        {
            QMutexLocker locker(&m_mutex);
            if (m_waitBuffer.isEmpty())
                m_hasTask.wait(&m_mutex);
            if (m_waitBuffer.isEmpty()) {
                break;  // destructor called
            }
            text = m_waitBuffer.takeFirst();
        }
        callWrite(text);
    }

    while (!m_waitBuffer.isEmpty()) {
        QString text = m_waitBuffer.takeFirst();
        callWrite(text);
    }
}

void Logger::add(const QString text)
{
    if (!m_enable)
        return;
    m_mutex.lock();
    m_waitBuffer.append(text);
    m_hasTask.wakeAll();
    m_mutex.unlock();
}

QString Logger::msgTypeToString(QtMsgType type)
{
    QString t;
    switch (type) {
    case QtDebugMsg:
        t = "debug   ";
        break;
    case QtInfoMsg:
        t = "info    ";
        break;
    case QtWarningMsg:
        t = "warning ";
        break;
    case QtCriticalMsg:
        t = "critical";
        break;
    case QtFatalMsg:
        t = "fatal   ";
        break;
    }

    return t;
}
