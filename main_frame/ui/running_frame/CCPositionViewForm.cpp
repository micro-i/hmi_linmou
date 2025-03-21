#include "CCPositionViewForm.h"
#include "ui_CCPositionViewForm.h"

CCPositionViewForm::CCPositionViewForm(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PositionViewForm)
{
    ui->setupUi(this);
}

CCPositionViewForm::~CCPositionViewForm()
{
    delete ui;
}

CCWorkpiece3d *CCPositionViewForm::getView3d()
{
    return ui->view3dwidget;
}
