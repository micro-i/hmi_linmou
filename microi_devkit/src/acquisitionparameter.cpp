#include "acquisitionparameter.h"

AcquisitionParameter::AcquisitionParameter(QObject *parent) : QObject(parent)
{
    this->params = { false, 0, nullptr, nullptr, nullptr, nullptr, 0, 0, 0, "", 0, 0, 0, 0, 0 };
    // 自定义类型如果想使用signal/slot来传递的话，必须注册，通常在构造函数里注册
    // 形式为: qRegisterMetaType<myclss.("myclass"), Myclass的引用类型需单独注册：qRegisterMetaType<MyClass>("Myclass&")
    qRegisterMetaType<AcquisitionParams>("AcquisitionParams");
}

AcquisitionParameter::~AcquisitionParameter()
{
}

void AcquisitionParameter::slot_updateParams(AcquisitionParams newParams) {
    this->params = newParams;
    emit updated(this->params);
}
