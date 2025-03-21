#include "testopenglform.h"
#include "ui_testopenglform.h"

#include <QOpenGLFunctions>

TestOpenglForm::TestOpenglForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestOpenglForm)
{
    ui->setupUi(this);
    this->layout()->addWidget(this->win2d);
}

TestOpenglForm::~TestOpenglForm()
{
    delete ui;
}
