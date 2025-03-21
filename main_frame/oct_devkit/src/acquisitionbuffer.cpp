#include "acquisitionbuffer.h"

AcquisitionBuffer::AcquisitionBuffer(QObject *parent)
{
    this->bufferCnt = 0;
    this->bytesPerBuffer = 0;
    this->bufferArray.resize(bufferCnt);
    this->bufferReadyArray.resize(bufferCnt);
    this->currIndex = -1;

    for (unsigned int i = 0; i < this->bufferCnt; i++) {
        this->bufferArray[i] = nullptr;
        this->bufferReadyArray[i] = false;
    }
}

AcquisitionBuffer::~AcquisitionBuffer() {
    releaseMemory();
}

bool AcquisitionBuffer::allocateMemory(unsigned int bufferCnt, size_t bytesPerBuffer) {
    this->bufferCnt = bufferCnt;
    this->bytesPerBuffer = bytesPerBuffer;
    this->releaseMemory();
    this->bufferArray.clear();
    this->bufferArray.resize(bufferCnt);
    this->bufferReadyArray.resize(bufferCnt);
    bool success = true;

    for (unsigned int bufferIndex = 0; (bufferIndex < this->bufferCnt) && success; bufferIndex++) {
        int err = posix_memalign((void**) &bufferArray[bufferIndex], 128, bytesPerBuffer);
        if (err != 0 || bufferArray[bufferIndex] == nullptr) {
            emit error("Buffer memory allocation error. posix_memalign() error code: " + QString::number(err));
            success = false;
        } else {
            // set bufferArray to 0, every buffersArray[] contains bytesPerBuffer bytes
            memset(bufferArray[bufferIndex], 1 , bytesPerBuffer);
        }
    }
    return success;
}

void AcquisitionBuffer::releaseMemory() {
    for (int i = 0; i < this->bufferArray.size(); i++) {
        if (this->bufferArray[i] != nullptr) {
            posix_memalign_free(this->bufferArray[i]);
            this->bufferArray[i] = nullptr;
            this->bufferReadyArray[i] = false;
        }
    }
}
