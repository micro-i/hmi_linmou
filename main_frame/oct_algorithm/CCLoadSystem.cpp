#include "CCLoadSystem.h"

#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QMessageBox>

#include "oct_devkit/src/plugin.h"
#include "CCSystemManager.h"

CCLoadSystem& CCLoadSystem::instance()
{
    static CCLoadSystem system;
    return system;
}

CCLoadSystem::CCLoadSystem()
{
    QDir pluginsDir = QDir(qApp->applicationDirPath());

    //check if plugins dir exists. if it does not exist change to the share_dev directory. this makes software development easier as plugins can be copied to the share_dev during the build process
    bool pluginsDirExists = pluginsDir.cd("systems");
    if(!pluginsDirExists){
        pluginsDir.mkdir("systems");
        QMessageBox::critical(nullptr, u8"找不到系统", u8"找不到任何系统，即将退出", QMessageBox::Ok);
        exit(-1);
    }
    m_systemPath = pluginsDir.absolutePath();
}

void CCLoadSystem::loadSystem()
{
    // 报错推出
    auto errorload = [](){
        QMessageBox::critical(nullptr, u8"错误，找不到任何系统", u8"找不到任何系统，即将退出", QMessageBox::Ok);
        exit(-1);
    };

    QDir pluginsDir = QDir(m_systemPath);
    QString msg;
    // 文件夹为空
    if(!pluginsDir.entryList(QDir::Files).size()){
        errorload();
    }
    // 遍历读取系统
    for(auto& fileName : pluginsDir.entryList(QDir::Files)) {
        {
            // TODO：读取系统时分别读取运行时与调试时，可以找出不用读取两遍的方法
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            //todo: figure out why qobject_cast<Plugin*>(loader.instance()) does not work and fix it
            QObject *plugin = loader.instance();
            bool isDebug = false;
            if(fileName.contains("Debug")) isDebug = true;
            if(!plugin){
                // 未识别插件
                msg = QString("Recognized dll file %1, but can not load it : %2").arg(fileName, loader.errorString());
                qDebug() << loader.errorString().toLatin1();
                emit error(msg);
                qWarning() << msg;
                continue;
            }
            if (plugin) {
                loader.instance();
                Plugin* actualPlugin = dynamic_cast<Plugin*>(plugin);
                if(!actualPlugin){
                    msg = QString("UnRecognized format of System : %1").arg(fileName);
                    emit error(msg);
                    qWarning() << msg;
                    continue;
                }
                enum PLUGIN_TYPE type = actualPlugin->getType();
                // 此处只读系统
                switch (type) {
                case SYSTEM:{
                    CCSystemManager::instance()->addSystem(qobject_cast<AcquisitionSystem*>(plugin), isDebug);
                    break;
                }
                default:{
                    qWarning() << QString("Could not load System : %1").arg(fileName);
                    emit error(msg);
                }
                }
            }
            // 未读入任何系统报错
            if(!CCSystemManager::instance()->getSystems().size()){
                errorload();
            }
        }
      }
}

