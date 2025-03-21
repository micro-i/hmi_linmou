#ifndef CCVOLUMECOLLECTOR_H
#define CCVOLUMECOLLECTOR_H

#include <QObject>
#include <QFile>
#include <QCoreApplication>
#include <QDateTime>

#include "oct_algorithm/oct_params/octalgorithmparameters.h"
#include "oct_params/Algorithm.Grpc.h"

class CCVolumeCollector : public QObject
{
	Q_OBJECT

public:
    static CCVolumeCollector *instance();
    CCVolumeCollector();
    ~CCVolumeCollector();
	
    bool m_collectorEnabled;
    bool m_collectorFinished;
    bool m_isCollecting;
private:
    char* m_collectBuffer;
    unsigned int m_collectedBuffers;
    bool m_initialized;
    OctAlgorithmParameters* m_params;
    unsigned int m_bufferSizeInBytes;

	void uninit();
    /**
     * @brief 发送给算法与保存到磁盘
     */
    void sendToAlgorithmAndDisk(unsigned int workpieceNr, unsigned int blockNr);
public slots :	
    void slot_init();
    void slot_abortRecording();
    void slot_collect(QVector<unsigned char> &buffer,
                      unsigned bitDepth,
                      unsigned int samplesPerLine,
                      unsigned int linesPerFrame,
                      unsigned int framesPerBuffer,
                      unsigned int buffersPerVolume,
                      unsigned int currentBufferNr,
                      unsigned int workpieceNr,
                      unsigned int blockNr);

signals :
	void info(QString info);
	void error(QString error);
    void collectingDone();
    void sendAIParam(AIParam param);
    void sendData(void* data, quint32 workpieceId, quint32 blockId);
};

#endif // CCVOLUMECOLLECTOR_H
