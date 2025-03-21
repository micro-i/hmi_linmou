#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>

namespace Ui {
class MainWidget;
}
// 设备状态
enum DEVICE_STATUS{
    DEVICE_SUCCESS,
    DEVICE_WARNING,
    DEVICE_ERROR
};
// 界面
enum FRAME{
    MAIN_FRAME,
    DATA_FRAME,
    DEBUG_FRAME
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private:
    /**
     * @brief 更改按钮风格
     */
    void changeFrame(enum FRAME);
    /**
     * @brief setDeviceStatus
     * @param status 状态
     * @param info 设备消息
     */
    void setDeviceStatus(enum DEVICE_STATUS status, QString info);
    /**
     * @brief setMask 设置遮罩显示
     * @param show
     */
    void setMask(bool show);
private slots:
    /**
     * @brief 更新当前时间
     */
    void slot_timeUpdate();
    /**
     * @brief 运行界面按钮点击
     */
    void slot_buttonMainFrame_clicked();
    /**
     * @brief 数据设置界面按钮点击
     */
    void slot_buttonData_clicked();
    /**
     * @brief 调试界面按钮点击
     */
    void slot_buttonDebug_clicked();
    /**
     * @brief 硬件状态按钮点击
     */
    void slot_buttonStatus_clicked();
    /**
     * @brief 设置按钮点击
     */
    void slot_buttonSettings_clicked();
    /**
     * @brief 切换角色按钮点击
     */
    void slot_buttonRole_clicked();
    /**
     * @brief 键盘敲击事件
     */
    void keyPressEvent(QKeyEvent*) override;

    void closeEvent(QCloseEvent *event) override;
private:
    Ui::MainWidget *ui;

    class CCRunningCombineForm* m_runningForm = nullptr;     // 运行界面
    class CCStatisticFrame* m_statisticForm = nullptr;       // 数据界面
    class OCTproZ* m_debugForm = nullptr;                    // 调试界面
    QWidget *m_maskWidget = nullptr;                         // mask控件
    QTimer m_systemTime;                                     // 当前时间计时器
};

#endif // MAINWIDGET_H
