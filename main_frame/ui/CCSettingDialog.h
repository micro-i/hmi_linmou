#ifndef CCSETTINGDIALOG_H
#define CCSETTINGDIALOG_H

#include <QDialog>
#include "ui/common/CCRadiusDialog.h"
#include <QVariantMap>

namespace Ui {
class CCSettingDialog;
}

class CCSettingDialog : public CCRadiusDialog
{
    Q_OBJECT

public:
    explicit CCSettingDialog(QWidget *parent = 0);
    ~CCSettingDialog();
    /**
     * @brief 从文件读取设置
     */
    void loadSettings();
    /**
     * @brief 保存设置到文件
     */
    void saveSettings();
    /**
     * @brief 更新处理参数到全局运行参数
     */
    void updateProcessingParams();
    /**
     * @brief 更新流参数到全局运行参数
     */
    void updateStreamingParams();
    /**
     * @brief 更新重采样参数
     */
    void updateResamplingParams();
    /**
     * @brief updateDispersionParams
     */
    void updateDispersionParams();
    /**
     * @brief updateWindowingParams
     */
    void updateWindowingParams();
private slots:
    void on_saveButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::CCSettingDialog *ui;

    QVariantMap processingSettings;
    QVariantMap streamingSettings;

private:
    /**
     * @brief 初始化标题栏
     */
    void initTitle();
    void showEvent(QShowEvent *) override;
};

#endif // CCSETTINGDIALOG_H
