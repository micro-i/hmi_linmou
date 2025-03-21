/**
  * @author ld
  * @brief  产品管理界面
  * @date   2022-12-30
  */
#ifndef CCPRODUCTMANAGEDIALOG_H
#define CCPRODUCTMANAGEDIALOG_H

#include "ui/common/CCRadiusDialog.h"   // parent

namespace Ui {
class CCProductManageDialog;
}

class CCProductManageDialog : public CCRadiusDialog
{
    Q_OBJECT

public:
    explicit CCProductManageDialog(QWidget *parent = nullptr);
    ~CCProductManageDialog();

    void connectDb();
private slots:
    // 保存
    void slot_saveButton_clicked();
    // 取消
    void slot_cancelButton_clicked();
    // 删除
    void slot_delButton_clicked();
    // 添加
    void slot_addButton_clicked();
private:
    Ui::CCProductManageDialog *ui;
    class QSqlTableModel* m_productModel;   // 数据库模型
    // 显示时间
    void showEvent(QShowEvent*) override;
};

#endif // CCPRODUCTMANAGEDIALOG_H
