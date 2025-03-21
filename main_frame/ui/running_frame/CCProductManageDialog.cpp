#include "CCProductManageDialog.h"
#include "ui_CCProductManageDialog.h"

#include <QSqlTableModel>
#include <QSqlError>
#include <QDebug>

#include "db/DbClientBase.h"
#include "infrastructure/CCSignalBridge.h"

CCProductManageDialog::CCProductManageDialog(QWidget *parent) :
    CCRadiusDialog(parent),
    ui(new Ui::CCProductManageDialog)
{
    ui->setupUi(this);

    connect(ui->addButton, &QPushButton::clicked, this, &CCProductManageDialog::slot_addButton_clicked);
    connect(ui->delButton, &QPushButton::clicked, this, &CCProductManageDialog::slot_delButton_clicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &CCProductManageDialog::slot_saveButton_clicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &CCProductManageDialog::slot_cancelButton_clicked);

    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->widget->setTitle(u8"产品管理");

    auto db = DbClientBase::instance()->sqlDatabase();
    if(!db.open()){
        qDebug() << "Db error";
    }

    m_productModel = new QSqlTableModel(this, db);
    m_productModel->setTable("product");
    connectDb();
}

CCProductManageDialog::~CCProductManageDialog()
{
    delete ui;
}

void CCProductManageDialog::connectDb()
{
    if(!m_productModel->select()){
        qDebug() << "Db error:" << m_productModel->lastError();
    }
    m_productModel->setHeaderData(0, Qt::Horizontal, u8"产品ID");
    m_productModel->setHeaderData(1, Qt::Horizontal, u8"产品名称");
    m_productModel->setHeaderData(2, Qt::Horizontal, u8"英文名称");
    m_productModel->setHeaderData(3, Qt::Horizontal, u8"备注");
    ui->tableView->setModel(m_productModel);
}

void CCProductManageDialog::slot_saveButton_clicked()
{
    bool bRet = m_productModel->submitAll();
    if(!bRet){
        ui->labelError->setText(m_productModel->lastError().text());
        return;
    }
    m_productModel->select();
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::accept();
}

void CCProductManageDialog::slot_cancelButton_clicked()
{
    m_productModel->revertAll();
    m_productModel->select();
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::reject();
}

void CCProductManageDialog::slot_addButton_clicked()
{
    int rowCount = m_productModel->rowCount();
    auto ret = m_productModel->insertRow(rowCount);
    if(!ret){
        ui->labelError->setText(m_productModel->lastError().text());
    }else{
        ui->labelError->clear();
    }
}

void CCProductManageDialog::slot_delButton_clicked()
{
    QModelIndex curIndex = ui->tableView->currentIndex();
    if(curIndex.isValid()){
        auto ret = m_productModel->removeRow(curIndex.row());
        if(!ret){
            ui->labelError->setText(m_productModel->lastError().text());
            return;
        }
    }
    ui->labelError->clear();
}

void CCProductManageDialog::showEvent(QShowEvent *)
{
    ui->labelError->clear();
}

