#include "CCSwitchRoleDialog.h"
#include "ui_CCSwitchRoleDialog.h"

#include "db/DbClientBase.h"                // get admin password
#include "infrastructure/CCSignalBridge.h"  // set mask

#include <QDebug>

CCSwitchRoleDialog::CCSwitchRoleDialog(QWidget *parent) :
    CCRadiusDialog(parent),
    ui(new Ui::CCSwitchRoleDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->widget->setTitle(u8"切换角色");
}

CCSwitchRoleDialog::~CCSwitchRoleDialog()
{
    delete ui;
}

void CCSwitchRoleDialog::on_pb_Confirm_clicked()
{
    QString password = DbClientBase::instance()->getUserConfig("admin_password");
    if(password.isNull() || password.isEmpty()){
        password = "00000000";
        DbClientBase::instance()->setUserConfig("admin_password", password);
    }
    if(m_faultTimes > 4){
        if(m_userTimer.elapsed() < 60000){
            ui->labelError->setText(QString(u8"密码错误超过五次,请%1秒后尝试!").arg((60000-m_userTimer.elapsed())/1000));
            ui->lineEdit->setStyleSheet("QLineEdit#lineEdit{border:1px solid red; border-radius:4px; background-color:#F7F8FA;}");
            return;
        }else{
           m_faultTimes = 0;
        }
    }
    if(!password.isNull() && !password.isEmpty() && password == ui->lineEdit->text()){
        m_role = m_role == 1 ? 0 : 1;
        static QString frontStyle = R"(QLabel#labelFrontIcon{
                             background-color: #3B5AFD;
                             border-radius: 4px;
                             background-image: url(:/images/user-%1-white.svg);
                             background-position:center;
                             background-repeat: none;
                             })";
        static QString backStyle = R"(QLabel#labelAfterIcon{
                            background-color: #F7F8FA;
                            border-radius: 4px;
                            background-image: url(:/images/user-%2.svg);
                            background-position:center;
                            background-repeat: none;
                            })";
        QString frontLabel = QString::fromLocal8Bit(R"(<html><head/><body><p><span style="color:#86909C;">%1</span><span style=" color:#3B5AFD;">(当前)</span></p></body></html>)");
        QString backLabel = R"(<html><head/><body><p><span style="color:#86909C;">%1</span></p></body></html>)";
        emit CCSignalBridge::instance()->setMask(false);
        QDialog::accept();
        if(m_role){
            ui->labelFrontIcon->setStyleSheet(frontStyle.arg("admin"));
            ui->labelAfterIcon->setStyleSheet(backStyle.arg("common"));
            ui->labelFront->setText(frontLabel.arg(u8"管理员"));
            ui->labelAfter->setText(backLabel.arg(u8"普通用户"));
        }else{
            ui->labelFrontIcon->setStyleSheet(frontStyle.arg("common"));
            ui->labelAfterIcon->setStyleSheet(backStyle.arg("admin"));
            ui->labelFront->setText(frontLabel.arg(u8"普通用户"));
            ui->labelAfter->setText(backLabel.arg(u8"管理员"));
        }
        emit sendRole(m_role);
        ui->lineEdit->clear();
    }else{
        ui->lineEdit->setStyleSheet("QLineEdit#lineEdit{border:1px solid red; border-radius:4px; background-color:#F7F8FA;}");
        if(m_faultTimes<5){
            if(0 == m_faultTimes) m_userTimer.restart();
            if(m_userTimer.elapsed() > 60000){
                m_userTimer.restart();
                m_faultTimes = 0;
            }
            m_faultTimes++;
            ui->labelError->setText(u8"密码错误!");
        }
        if(m_faultTimes>=5){
            ui->labelError->setText(u8"密码错误超过五次,一分钟后尝试!");
            m_userTimer.restart();
        }
    }
}

void CCSwitchRoleDialog::on_pb_Cancel_clicked()
{
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::reject(); 
}

void CCSwitchRoleDialog::on_lineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ui->labelError->clear();
    ui->lineEdit->setStyleSheet("QLineEdit#lineEdit{border:0px solid red; border-radius:4px; background-color:#F7F8FA;}");
}

