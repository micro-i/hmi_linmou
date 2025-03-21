/*********************************************************************
* @author        shenhuaibin
* @date          2022-11-30
* @brief         接受数据放入队列,线程转发到算法.并且通过信号外发算法处理结果
*********************************************************************/
#ifndef GRPCALGOPROXY
#define GRPCALGOPROXY
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

#include <grpcpp/grpcpp.h>
#include <grpc/algorithm.grpc.pb.h>

#include "oct_params/Algorithm.Grpc.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class GrpcAlgoProxy : public QThread
{
    Q_OBJECT
public:
    explicit GrpcAlgoProxy( std::shared_ptr<::grpc::Channel> channel, QObject *parent = nullptr);
    ~GrpcAlgoProxy();

    void appendQueueData(const AIParam& param);

    int modelId() const;
    /**
     * @brief setProxyNum 设置代理数量
     * @param num
     */
    static QVector<GrpcAlgoProxy*>& setProxyNum(int num);

    static GrpcAlgoProxy* getNextProxy();
    void stopRun();
signals:
    void inspectResultReady(const AIResult& results);

protected:
    virtual void run() override;
private:
    bool                                         m_isEnabled = true;
    QMutex                                       m_mutex;
    QWaitCondition                               m_bufferNotEmpty;
    QQueue <service_v2::InferenceRequest>        m_queueDatas; // 请求缓冲队列
    std::unique_ptr<service_v2::Inference::Stub> m_stub;
    int                                          m_modelId;
    static QVector<GrpcAlgoProxy*>               m_proxys;
};


#endif // GRPCALGOPROXY
