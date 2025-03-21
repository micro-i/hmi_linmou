#include "CCSystemManager.h"

CCSystemManager::CCSystemManager(QObject *parent) : QObject(parent)
{

}

CCSystemManager::~CCSystemManager()
{
    for(auto& system : m_systems){
//        if(system->acquisitionRunning)
//        system->stopAcquisition();
//        delete system;
    }
    for(auto& system : m_debugSystems){
//        if(system->acquisitionRunning)
//        system->stopAcquisition();
//        delete system;
    }
}

CCSystemManager * CCSystemManager::instance()
{
    static CCSystemManager manager;
    return &manager;
}

void CCSystemManager::addSystem(AcquisitionSystem* system, bool isDebug){
	if(system != nullptr){
        // 未知， 此处可能会出错(detach问题)
        QString name = system->getName();

        if(isDebug){
            if(!m_debugSystems.contains(name)){
                this->m_debugSystems[name] = system;
                qDebug() << QString("Success Load Debug System : %1").arg(name) << system;
            }else{
                delete system;
            }
        }else{
            if(!m_systems.contains(name)){
                this->m_systems[name] = system;
                qDebug() << QString("Success Load System : %1").arg(name) << system;
            }else{
                delete system;
            }
        }

	}
}


AcquisitionSystem* CCSystemManager::getSystemByName(QString name, bool isDebug){
    if(isDebug){
        if(m_debugSystems.contains(name)){
            return m_debugSystems[name];
        }
    }else{
        if(m_systems.contains(name)){
            return m_systems[name];
        }
    }
    // 不存在返回空
    return nullptr;
}

QList<AcquisitionSystem *> CCSystemManager::getSystems(bool isDebug)
{
    return isDebug? this->m_debugSystems.values() : this->m_systems.values();
}

QList<QString> CCSystemManager::getSystemNames(bool isDebug)
{
    return isDebug? this->m_debugSystems.keys() : this->m_systems.keys();
}
