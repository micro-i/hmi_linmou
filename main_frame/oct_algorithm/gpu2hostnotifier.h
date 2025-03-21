#ifndef GPU2HOSTNOTIFIER_H
#define GPU2HOSTNOTIFIER_H

#include <QObject>
#include <driver_types.h>   // for cuda

class Gpu2HostNotifier : public QObject
{
	Q_OBJECT

public:
    /**
     * @brief 获取调试通知器
     * @return
     */
    static Gpu2HostNotifier* getInstance();
	~Gpu2HostNotifier();
    /**
     * @brief Cuda回调
     * @param event
     * @param status
     * @param currStreamingBuffer
     */
	static void CUDART_CB dh2StreamingCallback(cudaStream_t event, cudaError_t status, void* currStreamingBuffer);

private:
    Gpu2HostNotifier(QObject *parent);          // 构造
public slots:
    /**
     * @brief 发送接收的数据
     * @param currStreamingBuffer
     */
    void emitCurrentStreamingBuffer(void* currStreamingBuffer);
signals:
    /**
     * @brief 录制结束
     * @param recordBuffer
     */
    void processedRecordDone(void* recordBuffer);
    /**
     * @brief 发送接收数据QVector版本
     * @param rawBuffer
     * @param bitDepth
     * @param samplesPerLine
     * @param linesPerFrame
     * @param framesPerBuffer
     * @param buffersPerVolume
     * @param currentBufferNr
     */
    void newGpuDataAvailibleV(QVector<unsigned char> &rawBuffer, unsigned bitDepth, unsigned int samplesPerLine,
                             unsigned int linesPerFrame, unsigned int framesPerBuffer,
                             unsigned int buffersPerVolume, unsigned int currentBufferNr);
    /**
     * @brief 发送接收数据
     * @param rawBuffer
     * @param bitDepth
     * @param samplesPerLine
     * @param linesPerFrame
     * @param framesPerBuffer
     * @param buffersPerVolume
     * @param currentBufferNr
     */
    void newGpuDataAvailible(void* rawBuffer, unsigned bitDepth, unsigned int samplesPerLine,
                             unsigned int linesPerFrame, unsigned int framesPerBuffer,
                             unsigned int buffersPerVolume, unsigned int currentBufferNr);
    /**
     * @brief 发送接收数据
     * @param rawBuffer
     * @param bitDepth
     * @param samplesPerLine
     * @param linesPerFrame
     * @param framesPerBuffer
     * @param buffersPerVolume
     * @param currentBufferNr
     */
    void newTaskGpuDataAvailible(QVector<unsigned char> &rawBuffer, unsigned bitDepth, unsigned int samplesPerLine,
                                 unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume,
                                 unsigned int currentBufferNr, unsigned int workPieceNr, unsigned int blockNr);
};


#endif //GPU2HOSTNOTIFIER_H
