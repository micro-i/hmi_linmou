#include "CCParamForm.h"
#include "ui_CCParamForm.h"

CCParamForm::CCParamForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CCParamForm)
{
    ui->setupUi(this);
}

CCParamForm::~CCParamForm()
{
    delete ui;
}
