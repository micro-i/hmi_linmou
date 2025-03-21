#include "gpu2hostnotifier.h"
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include <QThread>
#include <QDebug>

#include "oct_params/octalgorithmparameters.h"

Gpu2HostNotifier::Gpu2HostNotifier(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QVector<unsigned char>>("QVector<unsigned char>");
    qRegisterMetaType<QVector<unsigned char>>("QVector<unsigned char>&");
}

Gpu2HostNotifier* Gpu2HostNotifier::getInstance() {
    static Gpu2HostNotifier gpu2hostNotifier(nullptr);
    return &gpu2hostNotifier;
}

Gpu2HostNotifier::~Gpu2HostNotifier()
{
    qDebug() << "Gpu2HostNotifier destructor. Thread ID:" << QThread::currentThreadId();
}

void Gpu2HostNotifier::emitCurrentStreamingBuffer(void* streamingBuffer) {
    OctAlgorithmParameters* params = OctAlgorithmParameters::getInstanceForDebug();
    QVector<unsigned char> buffer(params->recParams.bufferSizeInBytes, 0);
    memcpy(&buffer[0], streamingBuffer, buffer.size());
    if(params->runMode){
        emit newTaskGpuDataAvailible(buffer, params->bitDepth, params->samplesPerLine / 2,
                                 params->ascansPerBscan, params->bscansPerBuffer, params->buffersPerVolume,
                                 params->currentBufferNr, params->workpieceID, params->blockNr);
    }else{
        emit newGpuDataAvailibleV(buffer, params->bitDepth, params->samplesPerLine / 2,
                                 params->ascansPerBscan, params->bscansPerBuffer, params->buffersPerVolume,
                                 params->currentBufferNr);
        emit newGpuDataAvailible(streamingBuffer, params->bitDepth, params->samplesPerLine / 2,
                                 params->ascansPerBscan, params->bscansPerBuffer, params->buffersPerVolume,
                                 params->currentBufferNr);
    }
}

void CUDART_CB Gpu2HostNotifier::dh2StreamingCallback(cudaStream_t event, cudaError_t status, void *currStreamingBuffer) {
    Q_UNUSED(event)
    checkCudaErrors(status);
    Gpu2HostNotifier::getInstance()->emitCurrentStreamingBuffer(currStreamingBuffer);
}

