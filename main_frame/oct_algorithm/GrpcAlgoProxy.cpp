#include "GrpcAlgoProxy.h"
#include <QCoreApplication>
#include <QtDebug>
#include <QBuffer>
#include <QTime>

QVector<GrpcAlgoProxy*> GrpcAlgoProxy::m_proxys;
int m_nextProxy = 0;

GrpcAlgoProxy::GrpcAlgoProxy( std::shared_ptr<::grpc::Channel> channel, QObject *parent )
    : QThread(parent)
{
    qRegisterMetaType<AIResult>("AIResult");
    m_stub = ::service_v2::Inference::NewStub(channel);
}

GrpcAlgoProxy::~GrpcAlgoProxy()
{
    stopRun();
    this->quit();
    this->wait();
}

void GrpcAlgoProxy::run()
{
    setTerminationEnabled();

    while (m_isEnabled)
    {
        ::service_v2::InferenceRequest queueData;

        {
            QMutexLocker locker(&m_mutex);
            if (m_queueDatas.isEmpty())
                m_bufferNotEmpty.wait(&m_mutex);
            if (!m_isEnabled) // destructor called
                break;
            queueData = m_queueDatas.dequeue();
        }

        ::service_v2::InferenceReply reply;
        ClientContext context;

        QTime t;
        t.start();
        Status status = m_stub->Inference(&context, queueData, &reply);
        int elapsed_time = t.elapsed();
        Q_UNUSED(elapsed_time);
        qDebug() << "GRPC STATUS : " << status.ok();
        AIResult aiResult;
        if (status.ok())
        {
            qDebug()<<QString("grpc call , product_id:%1 task_id:%2").arg(queueData.product_id()).arg(queueData.task_id());
            aiResult.m_productId	= reply.product_id ();
            aiResult.m_workpieceId  = reply.workpiece_id();
            aiResult.m_taskId  		= reply.task_id ();
            aiResult.m_widthId  	= reply.width_id ();
            aiResult.m_heightId  	= reply.height_id ();
            aiResult.m_depthId  	= reply.depth_id ();
            aiResult.m_numDetections= reply.num_detections ();

            for (int i=0;i<reply.singleinferencereply_size();i++)
            {
                aiResult.m_singleInferenceDatas.push_back(SingleInferenceData());
                SingleInferenceData& data = aiResult.m_singleInferenceDatas.back();

                ::service_v2::SingleInferenceReply defect=  reply.singleinferencereply(i);
                data.m_xMin 	=defect.xmin();
                data.m_yMin 	=defect.ymin();
                data.m_zMin 	=defect.zmin();
                data.m_xMax 	=defect.xmax();
                data.m_yMax 	=defect.ymax();
                data.m_zMax 	=defect.zmax();
                data.m_score    =defect.score();
                data.m_className = QString(defect.class_name().c_str());
                data.m_mask = QByteArray(defect.mask().c_str());

                int ptsize = defect.points_size();
                for(int j = 0; j < ptsize; j++)
                {
                    ::service_v2::Point pt = defect.points(j);
                    data.m_points.push_back(QVector3D(pt.x(),pt.y(),pt.z()));
                }
            };
            qDebug() << "get airesult";
            emit inspectResultReady(aiResult);

        } else {
            qCritical() << QString("%1 grpc server error, error code: %2, error message: %3, product_id: %4, task_id: %5")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(status.error_code()).arg(QString::fromStdString(status.error_message()))
                        .arg(queueData.product_id()).arg(queueData.task_id());
        }
    }

    qDebug() << QString("AlgorithmProxy [%1] quit").arg(m_modelId);
}

int GrpcAlgoProxy::modelId() const
{
    return m_modelId;
}

QVector<GrpcAlgoProxy*>& GrpcAlgoProxy::setProxyNum(int num)
{
    if(num == m_proxys.size()) return m_proxys;

    m_nextProxy = 0;
    if(m_proxys.size()){
        for(auto& proxy : m_proxys){
            proxy->stopRun();
            proxy->quit();
            proxy->wait();
            delete proxy;
        }
        m_proxys.clear();
    }

    std::string host = "192.168.100.18:10000";
    for(int i=0; i<num; i++){
        grpc::ChannelArguments channel_args;
        // set data to 4gigbyte minus 1 4294967295
        channel_args.SetMaxSendMessageSize(-1);
        channel_args.SetMaxReceiveMessageSize(-1);
        auto proxy = new GrpcAlgoProxy(::grpc::CreateCustomChannel(host, grpc::InsecureChannelCredentials(), channel_args));
        m_proxys.push_back(proxy);
        proxy->start();
    }
    return m_proxys;
}

GrpcAlgoProxy *GrpcAlgoProxy::getNextProxy()
{
    if(m_proxys.size()){
        m_nextProxy++;
        if(m_nextProxy >= m_proxys.size()) m_nextProxy = 0;  
        return m_proxys[m_nextProxy];
    }
    return nullptr;
}

void GrpcAlgoProxy::stopRun()
{
    m_isEnabled = false;
    m_bufferNotEmpty.wakeAll();
}

void GrpcAlgoProxy::appendQueueData(const AIParam& param)
{
    ::service_v2::InferenceRequest request;

    request.set_width_id  ( param.m_widthId  );
    request.set_height_id  ( param.m_heightId  );
    request.set_depth_id  ( param.m_depthId  );
    request.set_width  ( param.m_width  );
    request.set_height  ( param.m_height  );
    request.set_depth  ( param.m_depth  );
    request.set_encoded_image ( param.m_encodedImage.data(), param.m_encodedImage.size());
    request.set_product_id  ( param.m_productId  );
    request.set_task_id  ( param.m_taskId  );
    request.set_workpiece_id(param.m_workpieceId);
    request.set_dtype(param.m_dtype.toStdString());

    for(auto& mapData : param.m_features)
    {
        auto ftu_map = request.add_features()->mutable_feature();
        ::service_v2::Feature aFeature;

        for(QMap<QString,QVariant>::const_iterator itor = mapData.begin(); itor != mapData.end() ;++itor)
        {
            QVariant val = itor.value();
            if( QVariant::Int == val.type()) { aFeature.set_feature_int32(val.toInt()); }
            else if( QVariant::Double == val.type()) { aFeature.set_feature_float(val.toFloat()); }
            else if( QVariant::ByteArray == val.type()) {
                QByteArray ba = val.toByteArray();
                aFeature.set_feature(ba.data(), ba.size());
            }

            ftu_map->insert(::google::protobuf::MapPair<std::string,::service_v2::Feature>(itor.key().toStdString(),aFeature));
        }
    }

    m_mutex.lock();
    m_queueDatas.push_back(request);
    m_bufferNotEmpty.wakeAll();
    m_mutex.unlock();
}
