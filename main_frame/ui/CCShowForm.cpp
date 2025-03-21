#include "CCShowForm.h"
#include "ui_CCShowForm.h"

CCShowForm::CCShowForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CCShowForm)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_StyledBackground);
    ui->setupUi(this);
}

CCShowForm::~CCShowForm()
{
    delete ui;
}
