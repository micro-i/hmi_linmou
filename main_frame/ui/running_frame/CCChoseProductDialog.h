/**
  * @author ld
  * @brief  选择产品
  * @date   2022-12-30
  */
#ifndef CCCHOSEPRODUCTDIALOG_H
#define CCCHOSEPRODUCTDIALOG_H

#include "ui/common/CCRadiusDialog.h"   // parent
#include "db/DbModel.h"                 // db

namespace Ui {
class CCChoseProductDialog;
}

class CCChoseProductDialog : public CCRadiusDialog
{
    Q_OBJECT

public:
    explicit CCChoseProductDialog(QWidget *parent = 0);
    ~CCChoseProductDialog();
public:
    // TODO 此处看情况进行处理，通过函数返回*****
    Db::Product m_product;  // 产品
    QString m_batchNo;      // batchNo
private slots:
    // 点击确认
    void on_pb_Confirm_clicked();
    // 点击取消
    void on_pb_Cancel_clicked();
    // batchNo文字输入框，如果不输入更改样式
    void on_ldt_batchNo_textChanged(const QString &arg1);

private:
    Ui::CCChoseProductDialog *ui;
    QList<Db::Product> m_products; // 所有的产品
};

#endif // CCCHOSEPRODUCTDIALOG_H
