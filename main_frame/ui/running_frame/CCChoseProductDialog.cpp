#include "CCChoseProductDialog.h"
#include "ui_CCChoseProductDialog.h"

#include <QDebug>

#include "db/DbClientBase.h"

CCChoseProductDialog::CCChoseProductDialog(QWidget *parent) :
    CCRadiusDialog(parent),
    ui(new Ui::CCChoseProductDialog)
{
    ui->setupUi(this);
    ui->widget->setTitle(u8"产品信息");
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 添加产品信息
    m_products = DbClientBase::instance()->getProducts();
    for(auto& product : m_products){
        ui->cbx_Products->addItem(product.productName);
    }
}

CCChoseProductDialog::~CCChoseProductDialog()
{
    delete ui;
}

void CCChoseProductDialog::on_pb_Confirm_clicked()
{
    int idx = ui->cbx_Products->currentIndex();
    if(idx < 0){
        ui->cbx_Products->setStyleSheet("QLineEdit#ldt_batchNo{border:1px solid red; border-radius:4px; background-color:#F7F8FA;}");
        return;
    }
    if(idx > m_products.size()){
        QDialog::reject();
    }
    // 备注信息
    m_batchNo = ui->ldt_batchNo->text();
    on_ldt_batchNo_textChanged(m_batchNo);
    if(m_batchNo.isNull()){
        return;
    }
    // 选择产品
    m_product = m_products[idx];
    QDialog::accept();
}

void CCChoseProductDialog::on_pb_Cancel_clicked()
{
    QDialog::reject();
}

void CCChoseProductDialog::on_ldt_batchNo_textChanged(const QString &arg1)
{
    if(arg1.isNull()){
        // 加border
        ui->ldt_batchNo->setStyleSheet("QLineEdit#ldt_batchNo{border:1px solid red; border-radius:4px; background-color:#F7F8FA;}");
    }else{
        // 去除border
        ui->ldt_batchNo->setStyleSheet("QLineEdit#ldt_batchNo{border:0px solid red; border-radius:4px; background-color:#F7F8FA;}");
    }
}
