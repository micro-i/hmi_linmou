// qt
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QStyleFactory>
#include <QFile>
// vtk
#include <vtkOutputWindow.h>
#include <vtkSmartPointer.h>
#include <vtkStringOutputWindow.h>
// local
#include "ui/MainWidget.h"
#include "oct_algorithm/CCSystemManager.h"  // store system
#include "infrastructure/Logger.h"          // logger
#include "infrastructure/OCTGlobal.h"       // global variable

//#define TESTFONT
#ifdef  TESTFONT
#include <QFontDatabase>
#endif

#define SELF_ACQUI
#ifdef  SELF_ACQUI
#include "acquisition_system/oct_reconstruction_system/src/virtualoctsystem.h"
#include "acquisition_system/oct_regular_system/src/SDOctSystem.h"
#include "acquisition_system/oct_large_system_line/src/CCSDOctSystemL.h"
#else
#include "oct_algorithm/CCLoadSystem.h"
#endif

//#define USE_HELLO
#ifdef  USE_HELLO
#include "ui/CCShowForm.h"                  // load page
#endif

int main(int argc, char *argv[])
{
    // 启用属性
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
    QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // 让printf即时输出
    setvbuf(stdout, nullptr, _IONBF, 0);

    QApplication a(argc, argv);
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // TODO 全局样式表，未来从样式表中读取样式
    QFile style(":/qss/qss/global-light.qss");
    if(style.open(QIODevice::ReadOnly)){
        OCTGlobal::globalStyle = style.readAll();
        qApp->setStyleSheet(OCTGlobal::globalStyle);
    }

    // 消息重定向
    qInstallMessageHandler(Logger::messageWrapper);
    vtkSmartPointer<vtkStringOutputWindow> vtkOut = vtkSmartPointer<vtkStringOutputWindow>::New();
    vtkOut->SetUseStdErrorForAllMessages(true);
    vtkOutputWindow::SetInstance(vtkOut);

    //TEST 测试字体库
#ifdef TESTFONT
    QFontDatabase database;
    foreach (const QString &family, database.families())
    {
        qDebug() << family;
    }
#endif

    // 设置全局字体
    QFont f(QString::fromLocal8Bit("阿里巴巴普惠体"));
    qApp->setFont(f);
    a.setFont(f);

    // 读取采集系统
#ifdef SELF_ACQUI
    CCSystemManager::instance()->addSystem(new SDOCTSystem(), true);
    CCSystemManager::instance()->addSystem(new SDOCTSystem(), false);

    CCSystemManager::instance()->addSystem(new VirtualOCTSystem(), true);
    CCSystemManager::instance()->addSystem(new VirtualOCTSystem(), false);

    CCSystemManager::instance()->addSystem(new CCSDOctSystemL(), true);
    CCSystemManager::instance()->addSystem(new CCSDOctSystemL(), false);
#else
    CCLoadSystem::instance().loadSystem();
#endif
    // 选择采集系统
    QString systemName;
    if(qApp->arguments().contains("--debug")){
#ifdef SELF_ACQUI
        systemName = "Reconstruction OCT System";
#else
        systemName = "Reconstruction System";
#endif
        OCTGlobal::systemType = "debug";
    }else if(qApp->arguments().contains("--release")){
#ifdef SELF_ACQUI
        systemName = "Micro-i OCT System";
#else
        systemName = "SDOCT Acquisition System E2v";
#endif
        OCTGlobal::systemType = "debug";
    }
    else if(qApp->arguments().contains("--large")){
#ifdef SELF_ACQUI
        systemName = "Micro-i Large OCT System";
#else
        systemName = "Large field SDOCT Acquisition System";
#endif
        OCTGlobal::systemType = "large";
    }else{
#ifdef SELF_ACQUI
        systemName = "Micro-i Large OCT System";
#else
        systemName = "Large field SDOCT Acquisition System";
#endif
        OCTGlobal::systemType = "large";
    }

    OCTGlobal::currentSystem = CCSystemManager::instance()->getSystemByName(systemName);
    if(!OCTGlobal::currentSystem){
        QMessageBox::critical(nullptr, u8"错误，找不到任何系统", u8"找不到任何可用系统，即将退出", QMessageBox::Ok);
        exit(-1);
    }

    // 启动页
#ifdef USE_HELLO
    CCShowForm showForm;
    showForm.showMaximized();
    QEventLoop el;
    QTimer::singleShot(1000, &el, [&](){
        el.quit();
    });
    el.exec();
#endif
    // 主程序
    MainWidget w;
    w.showMaximized();

#ifdef USE_HELLO
    showForm.setVisible(false);
#endif

    auto ret = a.exec();
    return ret;
}
