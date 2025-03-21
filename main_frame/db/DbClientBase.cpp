#include "DbClientBase.h"

#include <QCoreApplication>
#include <QUuid>
#include <QDebug>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFile>

// static
uint64_t DbClientBase::m_counter = 0;
QMutex DbClientBase::m_mutex;

// global
QMutex globalGetMtx; //获取锁
static bool checkAndCreateTableFlag = false;    // 检查表是否存在

DbClientBase::DbClientBase(QObject *parent)
    : QObject(parent)
{
    // 读取配置
    QSettings settings("db.ini", QSettings::IniFormat);
    if (settings.value("host").isNull()) {
        settings.setValue("host", "127.0.0.1");
    }
    if (settings.value("name").isNull()) {
        settings.setValue("name", "hmi_oct");
    }
    settings.sync();
    // 准备数据
    QString host     = settings.value("host").toString();
    int     port     = 3306;
    QString name     = settings.value("name").toString();
    QString user     = "root";
    QString password = "Cube1234";
    // 初始化数据库
    initDb(host, user, password, port, name);
}

DbClientBase* DbClientBase::instance(){
    static DbClientBase db;
    if(!db.m_db.isValid()){
        if(!db.m_db.open()){
            qDebug() << "Db open error";
            return nullptr;
        }
    }
    return &db;
}

DbClientBase::DbClientBase(const QString& host, const QString& username, const QString& password,
                           const int port, const QString& dbname, QObject *parent)
    : QObject(parent)
{
    initDb(host, username, password, port, dbname);
}

DbClientBase::~DbClientBase()
{
    // 关闭写出线程
    m_dbWriteThread->stopThread();
    m_dbWriteThread = nullptr;

    if (m_db.isOpen()) {
        m_db.close();
    }
    qDebug() << "Db destructor. Thread ID: " << QThread::currentThread();
}

void DbClientBase::initDb(const QString& host, const QString& username, const QString& password,
                          const int port, const QString& dbname)
{   
    qRegisterMetaType<uint64_t>("uint64_t");
    // 数据库配置
    m_dbName = QUuid::createUuid().toString();
    m_db = QSqlDatabase::addDatabase("QMYSQL", m_dbName);
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbname);
    m_db.setUserName(username);
    m_db.setPassword(password);
    m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

    if (!m_db.open()) {
        qFatal("Can not open database: %s", m_db.lastError().text().toStdString().c_str());
        return;
    }

    if (!checkAndCreateTableFlag) {
        checkAndCreateTable();
    }

    m_mutex.lock();
    m_id = ++m_counter;
    m_mutex.unlock();

    m_dbWriteThread = std::unique_ptr<DbWriteThread>(new DbWriteThread(m_id, sqlDatabase()));
    connect(m_dbWriteThread.get(), &DbWriteThread::writeBufferSizeUpdated, this, &DbClientBase::onWriteBufferSizeUpdated);
    m_dbWriteThread->start();
    // 等待写入线程启动
    if(!m_dbWriteThread->isRunning()){
        QThread::sleep(1);
    }
    m_isValid = true;
}

void DbClientBase::checkAndCreateTable()
{
    QFile file(":/sql/HMI_OCT-DDL.sql");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Create table failed";
    }
    QString createTableSql = file.readAll();
    QSqlQuery query(m_db);
    if (query.exec(createTableSql)) {
        qDebug() << QString("Check and create table finished");
        // 检查创建表
        checkAndCreateTableFlag = true;
    } else {
        qWarning() << QString("Check and create table failed: %1").arg(query.lastError().text());
    }
}

void DbClientBase::setName(const QString &name)
{
    m_name = name;
}

bool DbClientBase::isValid()
{
    if(!m_db.isValid() || !m_db.isOpen()){
        if(!m_db.open()){
            m_isValid = false;
        }else{
            m_isValid = true;
        }
    }
    return m_isValid;
}

QSqlDatabase DbClientBase::sqlDatabase() const
{
    QSqlDatabase tmpDb;
    tmpDb = QSqlDatabase::addDatabase("QMYSQL", QUuid::createUuid().toString());
    tmpDb.setHostName(m_db.hostName());
    tmpDb.setPort(m_db.port());
    tmpDb.setDatabaseName(m_db.databaseName());
    tmpDb.setUserName(m_db.userName());
    tmpDb.setPassword(m_db.password());
    tmpDb.setConnectOptions("MYSQL_OPT_RECONNECT=1");
    // 返回一个同等设置的临时数据库
    return tmpDb;
}

quint32 DbClientBase::getLastTaskID() {
    quint32 taskid = 0;
    QString sql("SELECT task_id FROM task order by task_id desc limit 1");
    QMutexLocker locker(&globalGetMtx);
    QSqlQuery query(m_db);
    if (query.exec(sql)) {
        while(query.next()) {
            taskid = static_cast<quint32>(query.value(0).toInt());
        }
    }
    else {
        qWarning() << QString("Failed to excute sql(%1): %2").arg(sql, query.lastError().text());
    }
    return taskid;
}

QString DbClientBase::getUserConfig(QString configName)
{
    QString configValue;
    QString sql("SELECT setting_value FROM user where setting_name='%1';");
    QMutexLocker locker(&globalGetMtx);
    QSqlQuery query(m_db);
    if (query.exec(sql.arg(configName))) {
        while(query.next()) {
            configValue = query.value(0).toString();
            break;
        }
    }
    else {
        qWarning() << QString("Failed to excute sql(%1): %2").arg(sql, query.lastError().text());
    }
    return configValue;
}

QList<Db::Product> DbClientBase::getProducts()
{
    QString sql("SELECT * FROM product");
    QMutexLocker locker(&globalGetMtx);
    QSqlQuery query(m_db);
    QList<Db::Product> products;
    if(query.exec(sql)){
        QSqlRecord record = query.record();

        while(query.next()){
            QVariantMap map;
            for(int i=0; i<record.count(); i++){
                map.insert(record.fieldName(i), query.value(i));
            }
            Db::Product product;
            product.productId = static_cast<quint32>(map["product_id"].toInt());
            product.productName = map["name"].toString();
            product.productEnName = map["en_name"].toString();
            product.description = map["description"].toString();
            products.push_back(std::move(product));
        }
    }else{
        qWarning() << QString("Failed to excute sql(%1): %2").arg(sql, query.lastError().text());
    }
    return products;
}

void DbClientBase::writeProductModelInspectResult(const Db::ProductModelInspectResult& result)
{
    QString sql = QString("INSERT INTO product_model_inspect_result"
                          "(task_id, product_id, workpiece_id, sn, comment, defect_code, width, height, depth,"
                          "width_id, height_id, depth_id, dtype, time, x_min, y_min, z_min, x_max, y_max, z_max,"
                          "score, mask) VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12', '%13', '%14',"
                          "'%15', '%16', '%17', '%18', '%19', '%20', '%21', '%22');").arg(result.taskId).arg(result.productId).arg(result.workpieceId)
            .arg(result.sn, result.comment, result.defectCodes).arg(result.width).arg(result.height).arg(result.depth)
            .arg(result.widthId).arg(result.heightId).arg(result.depthId).arg(result.dtype, result.time).arg(result.xMin).arg(result.yMin)
            .arg(result.zMin).arg(result.xMax).arg(result.yMax).arg(result.zMax).arg(result.score).arg(result.mask);
    m_dbWriteThread->addTask(sql, m_id);
}

void DbClientBase::writeProductClassfiedResult(const Db::ProductClassfiedResult &result)
{
    QString sql = QString("INSERT INTO product_classfied_result(task_id, product_id, workpiece_id, sn, comment, defect_code_list, time)"
                          "VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7');").arg(result.taskId).arg(result.productId).arg(result.workpieceId)
            .arg(result.sn, result.comment, result.defectCodeList, result.time);
    m_dbWriteThread->addTask(sql, m_id);
}

void DbClientBase::setUserConfig(QString configName, QString value)
{
    QString sql = QString("INSERT INTO user(setting_name, setting_value)"
                          "VALUES('%1', '%2');").arg(configName, value);
    m_dbWriteThread->addTask(sql, m_id);
}

void DbClientBase::writeTask(const Db::Task& task)
{
    QString sql = QString("INSERT INTO task(task_id, product_id, comment, task_date, description)"
                          "VALUES(%1, %2, %3, now(), '%4');")
            .arg(task.taskId).arg(task.productId).arg(task.batchNo, task.description);
    m_dbWriteThread->addTask(sql, m_id);
}

void DbClientBase::onWriteBufferSizeUpdated(int size)
{
    emit writeBufferSizeUpdated(m_id, m_name, size);
}
