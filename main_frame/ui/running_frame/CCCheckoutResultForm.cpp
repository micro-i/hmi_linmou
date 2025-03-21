#include "CCCheckoutResultForm.h"
#include "ui_CCCheckoutResultForm.h"

#include "oct_algorithm/processing.h"           //绑定注册BUFFER槽函数
#include "ui/workpiece_frame/CCWorkpiece2d.h"   // 2d工件类

CCCheckoutResultForm::CCCheckoutResultForm(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CheckoutResultForm)
{
    ui->setupUi(this);
    // FIXME 此处重复设置屏幕数量会导致内存泄露，20221230
    setScreenNum(5, 5);
}

void CCCheckoutResultForm::setScreenNum(const int width, const int height){
    // 清除原先显示界面
    for(auto& frame: m_workPieceFrames){
        ui->gridLayout->removeWidget(frame);
        frame->disconnect();
        frame->setParent(nullptr);
        delete frame;
    }

    OctAlgorithmParameters::getInstanceForDebug()->totalScreenNum = static_cast<quint32>(width * height);
    OctAlgorithmParameters::getInstanceForDebug()->blockRowNum = static_cast<quint32>(height);
    OctAlgorithmParameters::getInstanceForDebug()->blockColNum = static_cast<quint32>(width);
    m_workPieceFrames.clear();

    // 初始化每个界面
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            int screenNum = i * height + j;
            CCWorkpiece2d* frame = new CCWorkpiece2d(screenNum, this);
            connect(frame, &CCWorkpiece2d::registerRunBufferCudaGL, Processing::getInstanceForRun(),
                    &Processing::slot_registerRunViewOpenGLbufferWithCuda, Qt::QueuedConnection);
            m_workPieceFrames.push_back(frame);
            ui->gridLayout->addWidget(frame, i, j, 1, 1);
        }
    }
}


CCCheckoutResultForm::~CCCheckoutResultForm()
{
    delete ui;
}


