#ifndef CCSWITCHROLEDIALOG_H
#define CCSWITCHROLEDIALOG_H

#include <QTime>
#include <QKeyEvent>

#include "ui/common/CCRadiusDialog.h" // set dialog radius

namespace Ui {
class CCSwitchRoleDialog;
}

class CCSwitchRoleDialog : public CCRadiusDialog
{
    Q_OBJECT

public:
    explicit CCSwitchRoleDialog(QWidget *parent = nullptr);
    ~CCSwitchRoleDialog() override;

signals:
    void sendRole(quint32 role);

private slots:
    void on_pb_Confirm_clicked();

    void on_pb_Cancel_clicked();

    void on_lineEdit_textChanged(const QString &arg1);
private:
    Ui::CCSwitchRoleDialog *ui;
    quint32 m_role = 0;
    QTime m_userTimer;
    quint8 m_faultTimes = 0;
};

#endif // CCSWITCHROLEDIALOG_H
