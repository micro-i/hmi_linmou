#include "CCControlPanel.h"
#include "ui_CCControlPanel.h"

CCControlPanel::CCControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CCControlPanel)
{
    ui->setupUi(this);
    connect(ui->cxb_enableCutter, &QCheckBox::stateChanged,
            this, &CCControlPanel::slot_enableStateChanged);
    connect(ui->cob_direction, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &CCControlPanel::slot_directionChanged);
    connect(ui->frameSlider, &QSlider::sliderMoved,
            this, &CCControlPanel::slot_frameSliderMoved);
    connect(ui->cxb_disable3D, &QCheckBox::stateChanged,
            this, &CCControlPanel::slot_disable3D);
}

CCControlPanel::~CCControlPanel()
{
    delete ui;
}

void CCControlPanel::setMaxIndex(int maxIndex)
{
    ui->frameSlider->setMaximum(maxIndex);
    ui->frameSlider->setMinimum(0);
//    ui->frameSlider->setValue(0);
}

void CCControlPanel::slot_enableStateChanged(int arg1)
{
    emit enableCutter(arg1);
}

void CCControlPanel::slot_disable3D(int state)
{
    emit disable3D(state);
}


void CCControlPanel::slot_directionChanged(int index)
{
    emit directionChanged(index);
}


void CCControlPanel::slot_frameSliderMoved(int position)
{
    emit frameChanged(position);
}

