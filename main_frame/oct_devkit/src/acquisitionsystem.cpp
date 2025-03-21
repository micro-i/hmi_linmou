
#include "acquisitionsystem.h"

AcquisitionSystem::AcquisitionSystem() : Plugin() {
	this->params = new AcquisitionParameter();
	this->buffer = new AcquisitionBuffer();
    this->acquisitionRunning = false;
}

AcquisitionSystem::~AcquisitionSystem(){
	qDebug() << "AcquisitionSystem destructor. Thread ID: " << QThread::currentThreadId();
	if (this->buffer != nullptr){delete this->buffer;}
	if (this->params != nullptr){delete this->params;}
}
