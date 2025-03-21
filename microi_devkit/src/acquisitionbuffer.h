#ifndef ACQUISITIONBUFFER_H
#define ACQUISITIONBUFFER_H

#include <QObject>
#include <QVector>
#include <QString>

// data alignment
#ifdef _WIN32
    #include <conio.h>
    #include <Windows.h>
    // posix_memalign(void** memptr, size_t alignment, size_t size) returns dynamic memory of (size) bytes if call successfully,
    // and its address is multiply times of (alignment), (alignment) is multiply times of void* type pointer as well as power exponent of 2
    // address is stored in (memptr), return0; return error code if not success
    #define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ? 0 : errno)
    // void aligned_free(void *palignedmem), release memoery pointed by palignedmem
    #define posix_memalign_free _aligned_free
#else
    #include <stdlib.h>
    #define posix_memalign_free free
#endif

class AcquisitionBuffer : public QObject
{
    Q_OBJECT
public:
    explicit AcquisitionBuffer(QObject *parent = nullptr);
    ~AcquisitionBuffer();

    bool allocateMemory(unsigned int bufferCnt, size_t bytesPerBuffer);
    void releaseMemory();

    QVector<void*>     bufferArray;          // buffer array to store acquisition data
    QVector<bool>      bufferReadyArray;     // flag that determines if current buffer data is transform to process/save and ready to be overwrite
    int                currIndex;            // current index of buffer for process/save
    unsigned int       bufferCnt;            // total numbers of buffer
    size_t             bytesPerBuffer;       // buffer size in byte

signals:
    void error(QString);
    void info(QString);

};

#endif // ACQUISITIONBUFFER_H
