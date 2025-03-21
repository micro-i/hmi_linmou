#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include "DbWriteThread.h"

DbWriteThread::DbWriteThread(uint64_t id, QSqlDatabase db)
    : QThread(),
      m_id(id),
      m_db(db)
{

}

DbWriteThread::~DbWriteThread()
{
    qDebug() << QString("Plugin Db write thread destroying [%1], wait task: %2").arg(m_id).arg(m_taskBuffer.size());
    this->stopThread();
    qDebug() << QString("Plugin Db write thread destroyed [%1]").arg(m_id);
}

void DbWriteThread::addTask(const QString &sql, quint64 id)
{
    if (!m_enabled)
        return;

    QMutexLocker locker(&m_mutex);
    m_taskBuffer.enqueue(Task(sql, id));
    emit writeBufferSizeUpdated(m_taskBuffer.size());
    m_hasTask.wakeAll();
}

void DbWriteThread::stopThread()
{
    m_enabled = false;
    m_hasTask.wakeAll();
    this->quit();
    this->wait();
    if(m_db.isOpen()) m_db.close();
}

void DbWriteThread::run()
{
    m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");
    if(!m_db.open()){
        qWarning() << m_db.lastError();
        return;
    }
    while (m_enabled) {
        Task task;
        {
            QMutexLocker locker(&m_mutex);
            if (m_taskBuffer.isEmpty()) {
                m_hasTask.wait(&m_mutex);
                if (!m_enabled)
                    break;
            }
            task = m_taskBuffer.dequeue();
            emit writeBufferSizeUpdated(m_taskBuffer.size());
        }

        QSqlQuery query(m_db);

        if (!query.exec(task.sql)) {
            qWarning() << QString("Failed to excute sql(%1): %2").arg(task.sql, query.lastError().text());
        }
    }

    qDebug() << QString("Db write thread [%1] quit").arg(m_id);
}
