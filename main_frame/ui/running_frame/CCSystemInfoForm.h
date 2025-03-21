/**
  * @author ld
  * @brief  系统信息界面，用于控制启停
  * @date   2022-11-28
  */

#ifndef SYSTEMINFOFORM_H
#define SYSTEMINFOFORM_H

#include <QFrame>
#include <QThread>
#include <QThreadPool>

namespace Ui {
class SystemInfoForm;
}

class CCSystemInfoForm : public QFrame
{
    Q_OBJECT

public:
    explicit CCSystemInfoForm(QWidget *parent = nullptr);
    ~CCSystemInfoForm();
    /**
     * @brief 更新进度条
     * @param cur 当前进度
     * @param total 总进度
     */
    void updateProgress(const int cur, const int total);
    /**
     * @brief 设置当前方案名称
     * @param name 方案名称
     */
    void setCurrentConfigName(const QString& name);
    /**
     * @brief disableStopButton 停止按钮启用
     */
    void enableStopButton();
signals:
    /**
     * @brief 开始采集
     */
    void start();
    /**
     * @brief 初始化
     */
    void init(quint32 taskId, quint32 productId, QString batchNo);
private slots:
    /**
     * @brief 启动按钮
     */
    void on_buttonPlay_clicked();
    /**
     * @brief 停止按钮
     */
    void on_buttonStop_clicked();
    /**
     * @brief 切换产品
     */
    void on_buttonProduct_clicked();

private:
    /**
     * @brief activateSystem 激活系统
     * @param system 系统名称
     */
    void activateSystem(class AcquisitionSystem* system);
    /**
     * @brief deactivateSystem 注销系统
     * @param system 系统名称
     */
    void deactivateSystem(class AcquisitionSystem* system);
    /**
     * @brief activeCurrentSystem 激活当前系统
     */
    void activeCurrentSystem();
    /**
     * @brief deactivateCurrentSystem 注销当前系统
     */
    void deactivateCurrentSystem();
private:
    Ui::SystemInfoForm *ui;
    // 采集线程
    QThread acquisitionThread;
};

#endif // SYSTEMINFOFORM_H
