#include "CCRunningCombineForm.h"
#include "ui_CCRunningCombineForm.h"

#include <QDateTime>

#include "oct_algorithm/processing.h"           // 数据处理
#include "CCSystemInfoForm.h"                   // 系统信息
#include "CCPositionViewForm.h"                 // 位置示意图
#include "CCDefectsStatisticsForm.h"            // 缺陷统计
#include "CCCheckoutResultForm.h"               // 检出结果
#include "ui/workpiece_frame/CCWorkpiece2d.h"   // 初始化检出结果界面多线程
#include "infrastructure/OCTGlobal.h"           // 全局变量获取
#include "db/DbClientBase.h"                    // 数据库操作
#include "oct_algorithm/GrpcAlgoProxy.h"        // GRPC协议


CCRunningCombineForm::CCRunningCombineForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RunningCombineForm)
{
    ui->setupUi(this);
    qRegisterMetaType<Db::Task>("Db::Task");
    // 添加运行界面分窗体
    m_checkoutResultForm = new CCCheckoutResultForm(this);
    m_systemInfoForm = new CCSystemInfoForm(this);
    m_positionViewForm = new CCPositionViewForm(this);
    m_defectStatisticsForm = new CCDefectsStatisticsForm(this);
    ui->vLayoutGroup->addWidget(m_systemInfoForm);
    ui->vLayoutGroup->addWidget(m_positionViewForm);
    ui->hLayoutGroup->addWidget(m_checkoutResultForm);
    ui->hLayoutGroup->addWidget(m_defectStatisticsForm);
    // INFO 更新运行参数
    this->m_octParams = OctAlgorithmParameters::getInstanceForDebug();
    // 运行时采集数据处理器
    m_signalProcessing = Processing::getInstanceForRun();
    // 消息输出
    connect(m_signalProcessing, &Processing::info, [](QString msg){
        qDebug() << msg;
    });
    connect(m_signalProcessing, &Processing::error, [](QString msg){
        qWarning() << msg;
    });
    // 初始化结束
    connect(m_systemInfoForm, &CCSystemInfoForm::init, this, &CCRunningCombineForm::init, Qt::DirectConnection);    // 直接初始化，非等待，所以直接连接
    connect(this, &CCRunningCombineForm::setCudaScreen, Processing::getInstanceForRun(), &Processing::setCudaScreen); // 设置显示窗口数量

    // opengl相关
    connect(this->m_signalProcessing, &Processing::initOpenGL, this, &CCRunningCombineForm::createOpenGLContextForProcessing);
    // task相关
    connect(this->m_signalProcessing, &Processing::updateScreenTaskInfo, this, &CCRunningCombineForm::updateScreenTaskInfo, Qt::QueuedConnection);
    connect(this->m_signalProcessing, &Processing::sendAIParam, this, &CCRunningCombineForm::updateTestResult);
    connect(this->m_signalProcessing, &Processing::sendData, this, &CCRunningCombineForm::update3dData, Qt::QueuedConnection);
    connect(this, &CCRunningCombineForm::sendDefect, m_defectStatisticsForm, &CCDefectsStatisticsForm::addDefect);

    connect(m_positionViewForm->getView3d(), &CCWorkpiece3d::registerBufferCudaGL, Processing::getInstanceForRun(), &Processing::slot_registerRunViewOpenGLbufferWithCuda, Qt::QueuedConnection);

    connect(this->m_signalProcessing, &Processing::initializationDone, m_systemInfoForm, &CCSystemInfoForm::enableStopButton);
    // GPU回调
    this->m_processedDataNotifier = Gpu2HostNotifier::getInstance();
    // WARNING 初始化运行
    initProcessing();
}

CCRunningCombineForm::~CCRunningCombineForm()
{
    m_calFuture.waitForFinished();
    delete ui;
    qDebug() << "CCRunningCombineForm destructor. Thread Id: " << QThread::currentThreadId();
}

void CCRunningCombineForm::initProcessing(){
    connect(OCTGlobal::currentSystem, &AcquisitionSystem::acquisitionStarted, m_signalProcessing, &Processing::slot_start);
    connect(OCTGlobal::currentSystem, &AcquisitionSystem::acquisitionStopped, this, &CCRunningCombineForm::slot_stop);
}

void CCRunningCombineForm::deinitProcessing(){

    disconnect(OCTGlobal::currentSystem, &AcquisitionSystem::acquisitionStarted, m_signalProcessing, &Processing::slot_start);
    disconnect(OCTGlobal::currentSystem, &AcquisitionSystem::acquisitionStopped, this, &CCRunningCombineForm::slot_stop);
}

void CCRunningCombineForm::createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread)
{
    qDebug() << "in create opengl";
    // 初始化界面
    m_positionViewForm->getView3d()->createOpenGLContextForProcessing(processingContext,processingSurface, processingThread);
    auto allFrames = m_checkoutResultForm->getAllFrames();
    auto size = allFrames.size();
    for(int i=0; i<size; i++){
        auto frame = allFrames[i];
        if(!frame){
            qDebug() << "Error Frame";
            continue;
        }
        // 通知处理器，注册CudaBuffer
        frame->createOpenGLContextForProcessing(processingContext, processingSurface, processingThread);
    }
    qDebug() << "Init Create End";
    //    emit contextInitDone();
}

void CCRunningCombineForm::init(quint32 taskId, quint32 productId, QString batchNo)
{
    m_defectStatisticsForm->clearTable();
    m_currentTask.batchNo = batchNo;
    m_currentTask.orderDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_currentTask.productId = productId;
    m_currentTask.taskId = taskId;
    DbClientBase::instance()->writeTask(m_currentTask);
    m_octParams->productID = productId; // 产品号
    m_octParams->taskID = taskId;   // 任务号
    // FIXME 设置总块数目，后续通过参数设置
    quint32 totalBlockNum = 25;
    m_octParams->volumesPerProduct = totalBlockNum;
    // FIXME 虚拟输出，后续通过按钮或者参数设置
    m_octParams->virtualResult = true;
    // 设置grpc proxy
    if(!m_octParams->virtualResult){
        auto proxys = GrpcAlgoProxy::setProxyNum(totalBlockNum);
        for(auto proxy : proxys){
            connect(proxy, &GrpcAlgoProxy::inspectResultReady, this, &CCRunningCombineForm::updateAlgorithmResult,
                    Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
        }
    }
    // 获取渲染大小信息
    unsigned int width = m_octParams->ascansPerBscan;
    unsigned int height = m_octParams->bscansPerBuffer * m_octParams->buffersPerVolume;
    unsigned int depth = m_octParams->samplesPerLine / 2;
    // 初始化界面
    auto allFrames = m_checkoutResultForm->getAllFrames();
    emit setCudaScreen(allFrames.size());
    auto size = allFrames.size();
    for(int i=0; i<size; i++){
        auto frame = allFrames[i];
        frame->initSize(width, height, depth);
    }
    m_positionViewForm->getView3d()->initSize(depth, width, height);
    // 强制更新参数, 运行参数
    m_octParams->useCustomResampleCurve = true;
    m_octParams->acquisitionParamsChanged = true;
    m_octParams->updateBufferSizeInBytes();
    m_octParams->updateResampleCurve();
    m_octParams->updateDispersionCurve();
    m_octParams->updateWindowCurve();
    qDebug() << "Init GL Size End";
}

void CCRunningCombineForm::updateScreenTaskInfo(quint32 screenIdx, quint32 workPieceId, quint32 blockNum, bool reverse)
{
    auto allFrames = m_checkoutResultForm->getAllFrames();
    auto size = allFrames.size();
    if(screenIdx >= size){
        qWarning() << "Could not find screen!";
        return;
    }
    auto frame = allFrames[screenIdx];

    frame->updateScreenInfo(workPieceId, blockNum);
    frame->setReverse(reverse);
    m_systemInfoForm->updateProgress(blockNum, m_octParams->volumesPerProduct);
}

void CCRunningCombineForm::updateTestResult(AIParam param)
{
    AIResult result;
    result.m_taskId = param.m_taskId;
    result.m_productId = param.m_productId;
    result.m_workpieceId = param.m_workpieceId;

//    result.m_numDetections = std::rand() % 2 ? 0 : 1;
    result.m_numDetections = 0;
    result.m_widthId = param.m_widthId;
    result.m_heightId = param.m_heightId;
    result.m_depthId = param.m_depthId;
    if(result.m_numDetections){
        SingleInferenceData singleDefect;
        singleDefect.m_xMin = 500;
        singleDefect.m_yMin = 300;
        singleDefect.m_zMin = 400;
        singleDefect.m_xMax = 100;
        singleDefect.m_yMax = 100;
        singleDefect.m_zMax = 100;
        static QStringList defects = {u8"油墨", u8"划伤", u8"气泡"};
        singleDefect.m_className = defects[std::rand() % 3];
        result.m_singleInferenceDatas.append(singleDefect);
    }
    updateAlgorithmResult(result);
}

void CCRunningCombineForm::updateAlgorithmResult(const AIResult &result)
{
    auto allFrames = m_checkoutResultForm->getAllFrames();
    for(auto& frame : allFrames){
        if(frame->compareScreen(result.m_workpieceId, result.m_widthId)){
            qDebug() << "Receive Result: " << result.m_widthId << ": " <<result.m_workpieceId
                     << ":" << result.m_depthId << ":" << result.m_heightId << ":" << result.m_numDetections
                     << ":" << result.m_taskId;
            m_calFuture = QtConcurrent::run([&]{
                Db::ProductModelInspectResult modelResult;
                modelResult.taskId = m_currentTask.taskId;
                modelResult.productId = m_currentTask.productId;
                modelResult.workpieceId = result.m_workpieceId;
                // TODO sn后续
                modelResult.sn = "";
                modelResult.comment = "";
                // TODO 此处需和发出参数一致
                modelResult.width = m_octParams->samplesPerLine / 2;
                modelResult.height = m_octParams->bscansPerBuffer * m_octParams->buffersPerVolume;
                modelResult.depth = m_octParams->ascansPerBscan;
                // TODO 此处可以用全局
                modelResult.dtype = "uint8";
                modelResult.widthId = result.m_widthId;
                modelResult.heightId = result.m_heightId;
                modelResult.depthId = result.m_depthId;
                modelResult.time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                modelResult.defectCodes = "";

                if(result.m_numDetections){
                    QList<QVector<float>> defects;
                    for(int i=0; i<result.m_numDetections; i++){
                        if(i >= result.m_singleInferenceDatas.size()){
                            qWarning() << "GRPC RESULT PARAMS NOT CONSISTENCY";
                            break;
                        }
                        emit sendDefect(result.m_widthId, result.m_singleInferenceDatas[i].m_className);

                        modelResult.xMin = result.m_singleInferenceDatas[i].m_xMin;
                        modelResult.yMin = result.m_singleInferenceDatas[i].m_yMin;
                        modelResult.zMin = result.m_singleInferenceDatas[i].m_zMin;
                        modelResult.xMax = result.m_singleInferenceDatas[i].m_xMax;
                        modelResult.yMax = result.m_singleInferenceDatas[i].m_yMax;
                        modelResult.zMax = result.m_singleInferenceDatas[i].m_zMax;
                        modelResult.score = result.m_singleInferenceDatas[i].m_score;
                        modelResult.defectCodes = result.m_singleInferenceDatas[i].m_className;
                        DbClientBase::instance()->writeProductModelInspectResult(modelResult);
                        defects.push_back({modelResult.xMin, modelResult.yMin, modelResult.zMin,
                                           modelResult.xMax, modelResult.yMax, modelResult.zMax});
                    }
                    frame->setDefects(defects);
                    frame->updateDefect(-1);
                }else{

                    frame->updateDefect(1);
                }

                // 更新工件
                if(m_currentWorkPiece.workpieceId != result.m_workpieceId){
                    if(-1 != m_currentWorkPiece.workpieceId){
                        DbClientBase::instance()->writeProductClassfiedResult(m_currentWorkPiece);
                    }else{
                        m_currentWorkPiece.taskId = m_currentTask.taskId;
                        m_currentWorkPiece.productId = m_currentTask.productId;
                        // TODO sn
                        m_currentWorkPiece.sn = "";
                        m_currentWorkPiece.comment = "";
                        m_currentWorkPiece.time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                    }
                    // 切换产品清空
                    m_currentWorkPiece.workpieceId = result.m_workpieceId;
                    m_currentWorkPiece.time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                    m_currentWorkPiece.defectCodeList = "";

                }

                for(int i=0; i<result.m_numDetections; i++){
                    if(i >= result.m_singleInferenceDatas.size()){
                        qWarning() << "GRPC RESULT PARAMS NOT CONSISTENCY";
                        break;
                    }
                    m_currentWorkPiece.defectCodeList += result.m_singleInferenceDatas[i].m_className + ",";
                }
            });
            m_calFuture.waitForFinished();
            return;
        }
    }
    qWarning() << "Could not find screen!";
}

void CCRunningCombineForm::update3dData(void *data, quint32 workpieceNr, quint32 blockNr)
{
    auto allFrames = m_checkoutResultForm->getAllFrames();
    for(auto& frame : allFrames){
        if(frame->compareScreen(workpieceNr, blockNr)){
            frame->setData(data, 8);
            return;
        }
    }
    free(data);
}

void CCRunningCombineForm::slot_stop()
{

}

