#include "CCVolumeCollector.h"

#include <QDebug>

#include "oct_algorithm/GrpcAlgoProxy.h"

CCVolumeCollector *CCVolumeCollector::instance()
{
    static CCVolumeCollector collector;
    return &collector;
}

CCVolumeCollector::CCVolumeCollector(){
    qRegisterMetaType<AIParam>("AIParam");

    this->m_collectorEnabled = false;
    this->m_collectorFinished = false;
    this->m_isCollecting = false;
    this->m_collectedBuffers = 0;
    this->m_collectBuffer = nullptr;
    this->m_initialized = false;
    this->m_params = OctAlgorithmParameters::getInstanceForDebug();
}

CCVolumeCollector::~CCVolumeCollector(){
    uninit();
    if(this->m_collectBuffer != nullptr){
        free(this->m_collectBuffer);
	}
//	qDebug() << "Recorder destructor. Thread ID: " << QThread::currentThreadId();
}

void CCVolumeCollector::slot_init(){
    if(this->m_collectBuffer){
        this->uninit();
    }
    unsigned int bytesPerSample = ceil((double)(m_params->bitDepth) / 8.0);
    m_bufferSizeInBytes = bytesPerSample * m_params->samplesPerLine * m_params->ascansPerBscan * m_params->bscansPerBuffer;
    m_bufferSizeInBytes = m_bufferSizeInBytes / 2; // TODO WHY????????
    this->m_collectBuffer = (char*)malloc(m_params->buffersPerVolume * m_bufferSizeInBytes);
    this->m_initialized = true;
    this->m_collectorFinished = false;
    this->m_collectorEnabled = true;
    this->m_isCollecting = false;
    emit info(tr("Recording initialized..."));
}

void CCVolumeCollector::slot_abortRecording()
{
//    if(this->recordingEnabled){
//        if (!this->recordingFinished) {
//            emit error(tr("Recording aborted!"));
//            this->recordingEnabled = false;
//            this->saveToDisk();
//            this->uninit();
//        }
//        return;
//    }
}

void CCVolumeCollector::uninit(){
    free(this->m_collectBuffer);
    this->m_collectBuffer = nullptr;
    this->m_initialized = false;
    this->m_collectorFinished = true;
    this->m_collectedBuffers = 0;
    emit collectingDone();
}

void CCVolumeCollector::slot_collect(QVector<unsigned char> &buffer,
                                     unsigned bitDepth,
                                     unsigned int samplesPerLine,
                                     unsigned int linesPerFrame,
                                     unsigned int framesPerBuffer,
                                     unsigned int buffersPerVolume,
                                     unsigned int currentBufferNr,
                                     unsigned int workpieceNr,
                                     unsigned int blockNr
                                     ){
    Q_UNUSED(bitDepth)
    Q_UNUSED(samplesPerLine)
    Q_UNUSED(linesPerFrame)
    Q_UNUSED(framesPerBuffer)
    // TODO 位深

    if (!this->m_collectorEnabled) {
		return;
	}
	//check if initialization was done
    if (!this->m_initialized) {
		emit error(tr("Recording not possible. Record buffer not initialized."));
		return;
	}

	//check if recording should start with first buffer of volume
    // TODO buffer collect;
//    if(!this->m_isCollecting && currentBufferNr != 0){
//        return;
//    }
    this->m_isCollecting = true;

	//record/copy buffer to current position in recBuffer
    void* recBufferPointer = &(this->m_collectBuffer[(this->m_collectedBuffers)*m_bufferSizeInBytes]);
    memcpy(recBufferPointer, &buffer[0], m_bufferSizeInBytes);
    this->m_collectedBuffers++;

	//stop recording if enough buffers have been recorded, save recBuffer to disk and release reBuffer memory
    if (this->m_collectedBuffers >= buffersPerVolume) {
        this->sendToAlgorithmAndDisk(workpieceNr, blockNr);
        // 重置准备发送下一次
        m_collectedBuffers = 0;
	}
}

void CCVolumeCollector::sendToAlgorithmAndDisk(unsigned int workpieceNr, unsigned int blockNr)
{
    if (!this->m_initialized) {
        emit error(tr("Record buffer not initialized."));
        return;
    }
    auto dataSize = m_params->buffersPerVolume * m_bufferSizeInBytes;
    emit info(tr("Captured buffers: ") + QString::number(this->m_collectedBuffers) + "/" + QString::number(m_params->buffersPerVolume));
    char * datatoAlgorithm = (char*)malloc(dataSize);
    memcpy(datatoAlgorithm, m_collectBuffer, dataSize);
    // datatoAlgorithm 发送给算法
    AIParam param;
    param.m_encodedImage.setRawData(datatoAlgorithm, dataSize);
    // TODO BLOCKNR TO W,H,D and ID
    param.m_widthId = blockNr;
    param.m_heightId = 1;
    param.m_depthId = 1;
    param.m_width = m_params->samplesPerLine / 2;
    param.m_height = m_params->ascansPerBscan;
    param.m_depth = m_params->bscansPerBuffer * m_params->buffersPerVolume;
    param.m_productId = m_params->productID;
    param.m_workpieceId = workpieceNr;
    param.m_taskId = m_params->taskID;
    param.m_dtype = "uint8";

    if(m_params->virtualResult){
        param.m_encodedImage.clear();
        emit sendData((void*)datatoAlgorithm, workpieceNr, blockNr);
        emit sendAIParam(param);
    }else{
        // 数据发送给Grpc
        auto proxy = GrpcAlgoProxy::getNextProxy();
        if(proxy){
            proxy->appendQueueData(param);
        }else{
            qWarning() << "GRPC Client Num Error";
        }
        // 数据发送给三维显示
        emit sendData((void*)datatoAlgorithm, workpieceNr, blockNr);
    }

    // 存到磁盘
    if(m_params->saveDataToFile){
        QString fileName = QString("./dat-%1-%2-%3-%4-%5.raw")
                .arg(m_params->productID).arg(m_params->taskID)
                .arg(workpieceNr).arg(blockNr).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        QFile outputFile(fileName);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            emit error(tr("Recording failed! Could not write file to disk."));
            return;
        }
        outputFile.write(m_collectBuffer, dataSize);
        outputFile.close();
    }
    emit info(tr("Data sended"));
}
