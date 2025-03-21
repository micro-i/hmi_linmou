/**
  * @author ld
  * @brief  运行界面集成
  * @date   2022-11-28
  */
#ifndef CCRUNNINGCOMBINEFORM_H
#define CCRUNNINGCOMBINEFORM_H

#include <QWidget>
#include <QThread>
#include <QtConcurrent>

#include "oct_algorithm/processing.h"
#include "db/DbModel.h"

namespace Ui {
class RunningCombineForm;
}

class CCRunningCombineForm : public QWidget
{
    Q_OBJECT

public:
    explicit CCRunningCombineForm(QWidget *parent = 0);
    ~CCRunningCombineForm();
    /**
     * @brief 初始化采集处理
     */
    void initProcessing();
    /**
     * @brief 注销采集处理
     */
    void deinitProcessing();
private:
    /**
     * @brief OPenGL界面注册多线程离屏渲染
     * @param processingContext
     * @param processingSurface
     * @param processingThread
     */
    void createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
    /**
     * @brief 初始化GL与产品
     */
    void init(quint32 taskId, quint32 productId, QString batchNo);
    /**
     * @brief updateScreenTaskInfo 更改屏幕信息
     * @param screenIdx
     * @param workPieceId
     * @param blockNum
     */
    void updateScreenTaskInfo(quint32 screenIdx, quint32 workPieceId, quint32 blockNum, bool reverse);
    /**
     * @brief updateTestResult 更新测试结果
     * @param param
     */
    void updateTestResult(AIParam param);
    /**
     * @brief updateAlgorithmResult 更新算法结果
     * @param result
     */
    void updateAlgorithmResult(const AIResult& result);

    void update3dData(void* data, quint32 workpieceNr, quint32 blockNr);
    /**
     * @brief slot_stop 采集系统停止
     */
    void slot_stop();
signals:
    // 通知处理进程初始化窗口结束
    void contextInitDone();
    // 通知Cuda有多少个窗口
    void setCudaScreen(int size);
    // 发送
    void sendDefect(int blockNr, QString defectName);
private:
    Ui::RunningCombineForm *ui;
    // 采集
    Processing*                     m_signalProcessing = nullptr;   // 采集处理
    QThread                         m_processingThread;             // 采集处理线程
    // oct参数
    class Gpu2HostNotifier*         m_processedDataNotifier;        // GPU回调器
    class OctAlgorithmParameters*   m_octParams;                    // 参数
    // 界面
    class CCCheckoutResultForm*     m_checkoutResultForm;           // 检出结果界面
    class CCSystemInfoForm*         m_systemInfoForm;               // 系统信息界面
    class CCPositionViewForm*       m_positionViewForm;             // 目前用来显示三维模型
    class CCDefectsStatisticsForm*  m_defectStatisticsForm;         // 缺陷统计
    // 数据
    Db::Task                        m_currentTask;                  // 当前处理任务
    Db::ProductClassfiedResult      m_currentWorkPiece;             // 当前工件
    // 多线程计算
    QFuture<void>                   m_calFuture;                    // 计算线程结束通知结果
};

#endif // CCRUNNINGCOMBINEFORM_H
