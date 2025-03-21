﻿#include "octproz.h"
#include "infrastructure/OCTGlobal.h"
#include "infrastructure/CCSignalBridge.h"
#include "oct_algorithm/oct_params/CCSystemSettings.h"

OCTproZ::OCTproZ(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OCTproZ)
{
    // ******数据注册
    qRegisterMetaType<AcquisitionParams>("AcquisitionParams");
    qRegisterMetaType<RecordingParams>("RecordingParams");
    qRegisterMetaType<size_t>("size_t");
    // ******消息输出框
    this->console = new MessageConsole(this);
    this->console->setObjectName("Message Output");
    this->dockConsole = new QDockWidget((tr("Message Output")), this);
    // ******系统选择
    this->sysChooser = new SystemChooser();
    this->currSystem = nullptr;
    this->currSystemName = "";
    // ******参数
    this->octParams = OctAlgorithmParameters::getInstanceForDebug();
    // ******扩展
    this->extManager = new ExtensionManager();
    // ******状态栏
    this->m_statusBar = new QStatusBar();
    // ******PLOT1D
    this->plot1D = new PlotWindow1D(this);
    connect(this, &OCTproZ::allowRawGrabbing, this->plot1D, &PlotWindow1D::slot_enableRawGrabbing);
    connect(this->plot1D, &PlotWindow1D::info, this->console, &MessageConsole::displayInfo);
    connect(this->plot1D, &PlotWindow1D::error, this->console, &MessageConsole::displayError);
    connect(this, &OCTproZ::linesPerBufferChanged, this->plot1D, &PlotWindow1D::slot_changeLinesPerBuffer);
    this->dock1D = new QDockWidget(tr("1D"), this);
    this->dock1D->setObjectName("1D");
    // ******BSCAN
    this->bscanWindow = new GLWindow2D(this);
    this->bscanWindow->setMarkerOrigin(TOP);
    this->bscanWindow->setName("bscan-window");
    this->enFaceViewWindow = new GLWindow2D(this);
    this->enFaceViewWindow->setMarkerOrigin(LEFT);
    this->enFaceViewWindow->setName("enfaceview-window");

    setStyleFormat(bscanWindow);
    setStyleFormat(enFaceViewWindow);
    setStyleFormat(sysChooser);

    // ******Marker
    connect(this->bscanWindow, &GLWindow2D::currentFrameNr, this->enFaceViewWindow, &GLWindow2D::setMarkerPosition);
    connect(this->enFaceViewWindow, &GLWindow2D::currentFrameNr, this->bscanWindow, &GLWindow2D::setMarkerPosition);
    //GL windows need to be closed to avoid linux bug where QFileDialog is not usable when a GL window is opend in background
    connect(this->bscanWindow, &GLWindow2D::dialogAboutToOpen, this, &OCTproZ::slot_closeOpenGLwindows);
    connect(this->enFaceViewWindow, &GLWindow2D::dialogAboutToOpen, this, &OCTproZ::slot_closeOpenGLwindows);
    connect(this->bscanWindow, &GLWindow2D::dialogClosed, this, &OCTproZ::slot_reopenOpenGLwindows);
    connect(this->enFaceViewWindow, &GLWindow2D::dialogClosed, this, &OCTproZ::slot_reopenOpenGLwindows);
    connect(this, &OCTproZ::glBufferTextureSizeBscan, this->bscanWindow, &GLWindow2D::changeTextureSize);
    this->dock2D = new QDockWidget(tr(u8"B扫描"), this);
    this->dock2D->setObjectName("2D - B-scan");
    //this->dock2D->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable); //make dock not floatable
    connect(this->dock2D, &QDockWidget::visibilityChanged, this, &OCTproZ::slot_enableBscanViewProcessing);


    connect(this, &OCTproZ::glBufferTextureSizeEnFaceView, this->enFaceViewWindow, &GLWindow2D::changeTextureSize);
    this->dockEnFaceView = new QDockWidget(tr(u8"C扫描"), this);
    this->dockEnFaceView->setObjectName("2D - En Face View");
    connect(this->dockEnFaceView, &QDockWidget::visibilityChanged, this, &OCTproZ::slot_enableEnFaceViewProcessing);
    // ******VOLUMESCAN
    this->volumeWindow = new GLWindow3D(this);
    this->volumeWindow->setName("3d-volume-window");
    this->dockVolumeView = new QDockWidget(tr(u8"三维扫描"), this);
    this->dockVolumeView->setObjectName("3D - Volume");
    //make dock not floatable, and not movable
    this->dockVolumeView->setFeatures(QDockWidget::DockWidgetClosable);
    connect(this, &OCTproZ::glBufferTextureSizeBscan, this->volumeWindow, &GLWindow3D::changeTextureSize);
    connect(this->dockVolumeView, &QDockWidget::visibilityChanged, this, &OCTproZ::slot_enableVolumeViewProcessing);
    //GL windows need to be closed to avoid linux bug where QFileDialog is not usable when a GL window is opend in background
    connect(this->volumeWindow, &GLWindow3D::dialogAboutToOpen, this, &OCTproZ::slot_closeOpenGLwindows);
    connect(this->volumeWindow, &GLWindow3D::dialogClosed, this, &OCTproZ::slot_reopenOpenGLwindows);

    // ******GL窗口状态
    //init bools that are used to save state of OpenGL windows to reopen them after QFileDialog was used on Linux
    this->isDock2DClosed = false;
    this->isDockEnFaceViewClosed = false;
    this->isDockVolumeViewClosed = false;
    // ******侧边栏
    this->sidebar = new Sidebar(this);
    this->sidebar->setObjectName("Sidebar");
    connect(this->sidebar, &Sidebar::info, this->console, &MessageConsole::displayInfo);
    connect(this->sidebar, &Sidebar::error, this->console, &MessageConsole::displayError);
    //GL windows need to be closed to avoid linux bug where QFileDialog is not usable when a GL window is opend in background
    connect(this->sidebar, &Sidebar::dialogAboutToOpen, this, &OCTproZ::slot_closeOpenGLwindows);
    connect(this->sidebar, &Sidebar::dialogClosed, this, &OCTproZ::slot_reopenOpenGLwindows);
    // ******采集器
    this->processingInThread = false;
    this->signalProcessing = new Processing(false);
#if defined(Q_OS_WIN) || defined(__aarch64__)
    this->processingInThread = true;
#elif defined(Q_OS_LINUX)
    this->signalProcessing->moveToThread(&processingThread);
    this->processingInThread = true;
    //todo: fix linux bug: opengl window seems to be laggy on ubuntu test system if signalProcessing is moved to thread and Virtual OCT System is used with small wait time (wait time that is set in the gui under "Wait after file read")
    //this->processingInThread = false;
#endif
    connect(CCSignalBridge::instance(), &CCSignalBridge::deactivateDebugSystem, this, &OCTproZ::slot_stop);
    connect(this, &OCTproZ::enableRecording, this->signalProcessing, &Processing::slot_enableRecording);
    connect(this->signalProcessing, &Processing::info, this->console, &MessageConsole::displayInfo);
    connect(this->signalProcessing, &Processing::error, this->console, &MessageConsole::displayError);
    connect(this->signalProcessing, &Processing::initializationDone, this, &OCTproZ::slot_enableStopAction);
    connect(this->signalProcessing, &Processing::streamingBufferEnabled, this->plot1D, &PlotWindow1D::slot_enableProcessedGrabbing);
    connect(this->signalProcessing, &Processing::rawData, this->plot1D, &PlotWindow1D::slot_plotRawData);
    connect(this->signalProcessing, &Processing::processedRecordDone, this, &OCTproZ::slot_resetGpu2HostSettings);
    connect(this->signalProcessing, &Processing::processedRecordDone, this, &OCTproZ::slot_recordingDone);
    connect(this->signalProcessing, &Processing::rawRecordDone, this, &OCTproZ::slot_recordingDone);
    connect(this->signalProcessing, &Processing::updateInfoBox, this, [&](QString vps, QString bps, QString bsps, QString aspa, QString bsz, QString dto){
        this->m_statusBar->showMessage(QString(u8"每秒块处理数量：%1, 每秒缓存处理数量:%2, 每秒B扫描数量:%3, 每秒A扫描数量:%4, 缓存大小:%5MB, 数据处理速度:%6MB/s")
                                       .arg(vps, 10).arg(bps, 10).arg(bsps, 10).arg(aspa, 10).arg(bsz, 10).arg(dto, 10));
    });
    // 设置通知
    connect(Settings::getInstance(), &Settings::info, this->console, &MessageConsole::displayInfo);
    connect(Settings::getInstance(), &Settings::error, this->console, &MessageConsole::displayError);
    //B-scan window connections:
    connect(this->bscanWindow->getControlPanel(), &ControlPanel2D::displayFrameSettingsChanged, this->signalProcessing, &Processing::slot_updateDisplayedBscanFrame);
    connect(this->bscanWindow, &GLWindow2D::registerBufferCudaGL, this->signalProcessing, &Processing::slot_registerBscanOpenGLbufferWithCuda);
    connect(this, &OCTproZ::registerBufferCudaGL, this->signalProcessing, &Processing::slot_registerBscanOpenGLbufferWithCuda);
    //En face view window connections:
    connect(this->enFaceViewWindow->getControlPanel(), &ControlPanel2D::displayFrameSettingsChanged, this->signalProcessing, &Processing::slot_updateDisplayedEnFaceFrame);
    connect(this->enFaceViewWindow, &GLWindow2D::registerBufferCudaGL, this->signalProcessing, &Processing::slot_registerEnFaceViewOpenGLbufferWithCuda);
    //Volume window connections:
    connect(this->volumeWindow, &GLWindow3D::registerBufferCudaGL, this->signalProcessing, &Processing::slot_registerVolumeViewOpenGLbufferWithCuda);
    //Processing connections:
    connect(this->signalProcessing, &Processing::initOpenGL, this->bscanWindow, &GLWindow2D::createOpenGLContextForProcessing);
    if(!this->processingInThread){
        connect(this->signalProcessing, &Processing::initOpenGL, this->enFaceViewWindow, &GLWindow2D::createOpenGLContextForProcessing); //due to opengl context sharing this connect is not necessary
    }
//    connect(this->signalProcessing, &Processing::initOpenGLenFaceView, this->bscanWindow, &GLWindow2D::registerOpenGLBufferWithCuda);
    connect(this->signalProcessing, &Processing::initOpenGLenFaceView, this->enFaceViewWindow, &GLWindow2D::registerOpenGLBufferWithCuda);
    connect(this->signalProcessing, &Processing::initOpenGLenFaceView, this->volumeWindow, &GLWindow3D::registerOpenGLBufferWithCuda);

    // WARNING 初始化调试
    // ******初始化窗口
    this->initGui();

    // ******GPU回调数据通知器
    this->processedDataNotifier = Gpu2HostNotifier::getInstance();
    connect(this->processedDataNotifier, &Gpu2HostNotifier::newGpuDataAvailible, this->plot1D, &PlotWindow1D::slot_plotProcessedData);
    // ******默认数据
    //default values to memorize stream2host setings
    this->streamToHostMemorized = this->octParams->streamToHost;
    this->streamingBuffersToSkipMemorized = octParams->streamingBuffersToSkip;
    //load saved settings from disc for main application
    ui->vLayoutStatus->addWidget(m_statusBar);
    this->loadSettings();
    this->loadSystemsAndExtensions();
//    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setStyleFormat(this);
    //    connect(qApp, &QCoreApplication::aboutToQuit, this, &OCTproZ::saveSettings); //todo: check if there is any difference between calling saveSettings via aboutToQuit signal and via the OCTproZ destructor
}

OCTproZ::~OCTproZ(){
    qDebug() << "OCTproZ destructor";
    this->slot_stop();
    acquisitionThread.quit();
    acquisitionThread.wait();

    delete ui;
    delete this->sysChooser;
    delete this->extManager;
    delete this->console;
    delete this->dockConsole;
    delete this->bscanWindow;
    delete this->enFaceViewWindow;
    delete this->dock2D;
    delete this->dockEnFaceView;
    delete this->dockVolumeView;
}

void OCTproZ::closeEvent(QCloseEvent* event) {
    this->saveSettings();

    //Stop acquisition if it is running
    if (this->currSystem != nullptr) {
        if (this->currSystem->acquisitionRunning) {
            this->slot_stop();
            QCoreApplication::processEvents(); //process events to ensure that acquisition is not running
            QThread::msleep(1000); //provide some time to let gpu computation finish
        }
    }
    event->accept();
}

void OCTproZ::initProcessing(){
}

void OCTproZ::deinitProcessing(){
}

void OCTproZ::initActionsAndDocks() {
    //this->viewToolBar->setIconSize(QSize(64,64));
    this->controlToolBar = new QToolBar(tr("Control Toolbar"));
    this->controlToolBar->setObjectName("Control Toolbar");
    this->controlToolBar->setVisible(true);

    //init 2d view tools toolbar
    QAction* bscanMarkerAction = this->bscanWindow->getMarkerAction();
    QAction* enfaceMarkerAction = this->enFaceViewWindow->getMarkerAction();
    QAction* bscanCoordAction = this->bscanWindow->getCoordAction();
    QAction* enfaceCoordAction = this->enFaceViewWindow->getCoordAction();
    bscanMarkerAction->setIcon(QIcon(":/icons/octproz_bscanmarker_icon.png"));
    bscanMarkerAction->setToolTip(tr("Display en face view position marker in B-scan"));
    enfaceMarkerAction->setIcon(QIcon(":/icons/octproz_enfacemarker_icon.png"));
    enfaceMarkerAction->setToolTip(tr("Display B-scan position marker in en face view"));

    ui->gridLayout->addWidget(this->sidebar->getDock());

    this->dockConsole->setFloating(false);
    this->dockConsole->setVisible(true);
    this->dockConsole->setTitleBarWidget(new QWidget()); //this removes title bar

    this->actionStart = new QAction("Start", this);
    this->actionStart->setIcon(QIcon(":/icons/octproz_play_icon-white.png"));

    this->actionStop = new QAction("Stop", this);
    this->actionStop->setIcon(QIcon(":/icons/octproz_stop_icon-white.png"));

    this->actionRecord = new QAction("Rec", this);
    this->actionRecord->setIcon(QIcon(":/icons/octproz_record_icon-white.png"));

    this->actionSelectSystem = new QAction("Open System", this);
    this->actionSelectSystem->setIcon(QIcon(":/icons/octproz_connect_icon-white.png"));

    this->actionSystemSettings = new QAction("System Settings", this);
    this->actionSystemSettings->setIcon(QIcon(":/icons/octproz_settings_icon-white.png"));
    this->actionSystemSettings->setStatusTip(tr("Settings of the currently loaded OCT system"));

    this->prepareDockWidget(this->dock1D, this->plot1D, this->action1D, QIcon(":/icons/octproz_rawsignal_icon.png"), "1D");
    this->prepareDockWidget(this->dock2D, this->bscanWindow, this->action2D, QIcon(":/icons/octproz_bscan_icon.png"), "2D - B-scan");
    this->dock2D->setFloating(false);
    this->dock2D->setVisible(true);

    this->prepareDockWidget(this->dockEnFaceView, this->enFaceViewWindow, this->actionEnFaceView, QIcon(":/icons/octproz_enface_icon.png"), "2D - En Face View");
    this->dockEnFaceView->setFloating(false);
    this->dockEnFaceView->setVisible(true);
    //this->dockEnFaceView->setLayoutDirection(Qt::Horizontal);

    this->prepareDockWidget(this->dockVolumeView, this->volumeWindow, this->action3D, QIcon(":/icons/octproz_volume_icon.png"), "3D - Volume");
    this->dockVolumeView->setFloating(false);
    this->dockVolumeView->setVisible(true);
    this->dockVolumeView->setMinimumWidth(320);

    this->prepareDockWidget(this->dockConsole, this->console, this->actionConsole, QIcon(":/icons/octproz_log_icon.png"), "Console");
    //	this->addDockWidget(Qt::BottomDockWidgetArea, this->dockConsole);
    ui->gLayoutOutput->addWidget(this->dockConsole);


    this->dockConsole->setFloating(false);
    this->dockConsole->setVisible(true);
    this->dockConsole->setTitleBarWidget(new QWidget()); //this removes title bar

    connect(this->actionStart, &QAction::triggered, this, &OCTproZ::slot_start);
    connect(this->actionStop, &QAction::triggered, this, &OCTproZ::slot_stop);
    connect(this->actionRecord, &QAction::triggered, this, &OCTproZ::slot_record);
    connect(this->actionSelectSystem, &QAction::triggered, this, &OCTproZ::slot_selectSystem);
    connect(this->actionSystemSettings, &QAction::triggered, this, &OCTproZ::slot_menuSystemSettings);

    this->actionStart->setEnabled(false);
    this->actionStop->setEnabled(false);
    this->actionRecord->setEnabled(false);

    this->controlToolBar->addAction(this->actionStart);
    this->controlToolBar->addAction(this->actionStop);
    this->controlToolBar->addAction(this->actionRecord);
    this->controlToolBar->addSeparator();
    this->controlToolBar->addAction(this->actionSelectSystem);
    this->controlToolBar->addAction(this->actionSystemSettings);
    this->controlToolBar->addSeparator();
    this->controlToolBar->addAction(this->action1D);
    this->controlToolBar->addAction(this->action2D);
    this->controlToolBar->addAction(this->actionEnFaceView);
    this->controlToolBar->addAction(this->action3D);
    this->controlToolBar->addSeparator();
    this->controlToolBar->addAction(bscanMarkerAction);
    this->controlToolBar->addAction(enfaceMarkerAction);
    this->controlToolBar->addAction(bscanCoordAction);
    this->controlToolBar->addAction(enfaceCoordAction);

    ui->vLayoutTool->addWidget(this->controlToolBar);
}

void OCTproZ::initGui() {
    ui->setupUi(this);
    this->initActionsAndDocks(); //initActionsAndDocks() must be called before initMenu()
    this->sidebar->init(this->actionStart, this->actionStop, this->actionRecord, this->actionSelectSystem, this->actionSystemSettings);
    this->initMenu();
    this->forceUpdateProcessingParams();

    //Message Console connects
    connect(this, &OCTproZ::info, this->console, &MessageConsole::displayInfo);
    connect(this, &OCTproZ::error, this->console, &MessageConsole::displayError);
    connect(this->bscanWindow, &GLWindow2D::info, this->console, &MessageConsole::displayInfo);
    connect(this->bscanWindow, &GLWindow2D::error, this->console, &MessageConsole::displayError);
    connect(this->enFaceViewWindow, &GLWindow2D::info, this->console, &MessageConsole::displayInfo);
    connect(this->enFaceViewWindow, &GLWindow2D::error, this->console, &MessageConsole::displayError);
    connect(this->volumeWindow, &GLWindow3D::info, this->console, &MessageConsole::displayInfo);
    connect(this->volumeWindow, &GLWindow3D::error, this->console, &MessageConsole::displayError);

    //Connects to bypass Linux/Qt bugs
    connect(this, &OCTproZ::closeDock2D, this, &OCTproZ::slot_closeOpenGLwindows);
    connect(this, &OCTproZ::reopenDock2D, this, &OCTproZ::slot_reopenOpenGLwindows);
}

void OCTproZ::prepareDockWidget(QDockWidget*& dock, QWidget* widgetForDock, QAction*& action, const QIcon& icon, QString iconText) {
    dock->setWidget(widgetForDock);
    //	this->addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);
    ui->hLayoutRight->addWidget(dock);
    dock->setVisible(false);
    dock->setFloating(true);
    action = dock->toggleViewAction();
    if(!icon.isNull()){
        action->setIcon(icon);
    }
    action->setIconText(iconText);
    this->controlToolBar->addAction(action);
}

void OCTproZ::initMenu() {
    //file menu
//    this->actionSelectSystem->setShortcut(QKeySequence::Open);
//    this->actionSystemSettings->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
//    QMenu *fileMenu = ui->menuBar->addMenu(tr("&File"));
//    fileMenu->addAction(this->actionSelectSystem);
//    fileMenu->addAction(this->actionSystemSettings);
//    fileMenu->addSeparator();
//    setStyleFormat(fileMenu);

    //view menu
//    QMenu* viewMenu = ui->menuBar->addMenu(tr("&View"));
    //get view toolbar
//    QAction* viewToolBarAction = this->viewToolBar->toggleViewAction();
//    viewToolBarAction->setText(tr("Show View Toolbar"));
    //get 2d tools toolbar
//    QAction* view2DExtrasToolBarAction = this->view2DExtrasToolBar->toggleViewAction();
//    view2DExtrasToolBarAction->setText(tr("Show 2D View Tools"));
    //get control toolbar
    QAction* controlToolBarAction = this->controlToolBar->toggleViewAction();
    controlToolBarAction->setText(tr("Show Control Toolbar"));
    //add toolbar view actions to view menu
//    viewMenu->addActions({ viewToolBarAction, view2DExtrasToolBarAction, controlToolBarAction, this->action1D, this->action2D, this->actionEnFaceView, this->action3D, this->actionConsole });

    //extras menu
    this->extrasMenu = ui->menuBar->addMenu(tr("&Extras"));
    QMenu* klinMenu = this->extrasMenu->addMenu(tr("&Resampling curve for k-linearization"));
    klinMenu->setToolTipsVisible(true);
    klinMenu->setStatusTip(tr("Settings for k-linearization resampling curve"));
    klinMenu->setIcon(QIcon(":/icons/octproz_klincurve_icon.png"));
    QActionGroup* customKlinCurveGroup = new QActionGroup(this);
    this->actionSetCustomKLinCurve = new QAction(tr("&Load custom curve from file..."), this);
    connect(this->actionSetCustomKLinCurve, &QAction::triggered, this, &OCTproZ::slot_loadCustomResamplingCurve);
    this->actionUseSidebarKLinCurve = new QAction(tr("Use &polynomial curve from sidebar"), this);
    this->actionUseCustomKLinCurve = new QAction(tr("Use &custom curve from file"), this);
    this->actionUseSidebarKLinCurve ->setCheckable(true);
    this->actionUseCustomKLinCurve->setCheckable(true);
    this->actionUseSidebarKLinCurve->setActionGroup(customKlinCurveGroup);
    this->actionUseCustomKLinCurve->setActionGroup(customKlinCurveGroup);
    this->actionUseCustomKLinCurve->setEnabled(false);
    this->actionUseSidebarKLinCurve->setChecked(true);
    connect(this->actionUseCustomKLinCurve, &QAction::toggled, this, &OCTproZ::slot_useCustomResamplingCurve);
    klinMenu->addAction(this->actionUseSidebarKLinCurve);
    klinMenu->addAction(this->actionUseCustomKLinCurve);
    QAction* klinSeparator = klinMenu->addSeparator();
    klinMenu->addAction(this->actionSetCustomKLinCurve);
    QList<QAction*> klinActions;
    klinActions << this->actionUseSidebarKLinCurve << this->actionUseCustomKLinCurve << klinSeparator <<this->actionSetCustomKLinCurve; //todo: move k-linearization actions to sidebar class as well as loadResamplingCurveFromFile method. Get the actions from the sidebar to create the extras menu of main window. Save if custom curve is used and add auto loading at startup
    this->sidebar->addActionsForKlinGroupBoxMenu(klinActions);
}

void OCTproZ::initExtensionsMenu() {
    QMenu* extensionMenu = this->extrasMenu->addMenu(tr("&Extensions"));
    extensionMenu->setIcon(QIcon(":/icons/octproz_extensions_icon.png"));

    auto extensionNames = this->extManager->getExtensionNames();
    foreach(QString extensionName, extensionNames) {
        QAction* extAction = extensionMenu->addAction(extensionName, this, &OCTproZ::slot_menuExtensions);
        this->extensionActions.append(extAction);
        extAction->setCheckable(true);
        extAction->setChecked(false);
        Extension* extension = this->extManager->getExtensionByName(extensionName);
        QString extensionToolTip = extension == nullptr ? "" : extension->getToolTip(); //todo: error handling if extension is nullptr
        extAction->setStatusTip(extensionToolTip);
    }
}

void OCTproZ::loadSystemsAndExtensions() {
    QDir pluginsDir = QDir(qApp->applicationDirPath());

    //check if plugins dir exists. if it does not exist change to the share_dev directory. this makes software development easier as plugins can be copied to the share_dev during the build process
    bool pluginsDirExists = pluginsDir.cd("plugins");
    if(!pluginsDirExists){
        qDebug() << "Error path: " << pluginsDir;
    }

    for(auto fileName : pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance(); //todo: figure out why qobject_cast<Plugin*>(loader.instance()) does not work and fix it
        if(!plugin){
            qDebug() << loader.errorString();
        }
        qDebug() << fileName << " : " << plugin;
        if (plugin) {
            Plugin* actualPlugin = (Plugin*)(plugin);
            enum PLUGIN_TYPE type = actualPlugin->getType();
            connect(actualPlugin, &Plugin::setKLinCoeffsRequest, this->sidebar, &Sidebar::slot_setKLinCoeffs); //Experimental! May be removed in future versions.
            connect(actualPlugin, &Plugin::setDispCompCoeffsRequest, this->sidebar, &Sidebar::slot_setDispCompCoeffs); //Experimental! May be removed in future versions.
            connect(this->sidebar, &Sidebar::klinCoeffs, actualPlugin, &Plugin::setKLinCoeffsRequestAccepted); //Experimental! May be removed in future versions.
            connect(this->sidebar, &Sidebar::dispCompCoeffs, actualPlugin, &Plugin::setDispCompCoeffsRequestAccepted); //Experimental! May be removed in future versions.
            connect(actualPlugin, &Plugin::startProcessingRequest, this, &OCTproZ::slot_start); //Experimental! May be removed in future versions.
            connect(actualPlugin, &Plugin::stopProcessingRequest, this, &OCTproZ::slot_stop); //Experimental! May be removed in future versions.
            qDebug() << type;
            switch (type) {
                case EXTENSION:{
                    Extension* extension = qobject_cast<Extension*>(plugin);
                    this->extManager->addExtension(extension);
                    if(extension->getDisplayStyle() == SEPARATE_WINDOW){
                        //init extension window
                        QWidget* extensionWidget = extension->getWidget();
                        extensionWidget->setParent(this); //this is necessary to automatically close all open extension windows when main application is closed
                        extensionWidget->setWindowFlags(Qt::Window);
                        extensionWidget->setAttribute(Qt::WA_DeleteOnClose, false);
                        extensionWidget->setWindowTitle(extension->getName());
                        ExtensionEventFilter* extensionCloseSignalForwarder = new ExtensionEventFilter(extensionWidget); //setting extensionWidget as parent causes ExtensionEventFilter object to be destroyed when extensionWidget gets destroyed
                        extensionCloseSignalForwarder->setExtension(extension);
                        extensionWidget->installEventFilter(extensionCloseSignalForwarder);
                        connect(extensionCloseSignalForwarder, &ExtensionEventFilter::extensionWidgetClosed, this, &OCTproZ::slot_uncheckExtensionInMenu); //this connection is used to automatically uncheck extension in menu if user closes a separate window extension by clicking on x
                    }
                    break;
                }
                default: {
                    emit error(tr("Could not load Plugin"));
                }
            }
        }
    }
    if (this->extManager->getExtensions().size() > 0) {
        this->initExtensionsMenu();
        qDebug() << "Innnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
    }
}

void OCTproZ::updateSiderBar()
{
    this->sidebar->updateProcessingParams();
    this->sidebar->updateResamplingParams();
    this->sidebar->updateWindowingParams();
}

void OCTproZ::setStyleFormat(QWidget* widget, bool origin)
{
    if(origin){
        widget->setStyleSheet(
                    "QWidget{background-color: #FFFFFF; color: #000000;}"
                    //reduce separator width
                    "QWidget::separator {width: 1px} "
                    //adjust menu style
                    "QMenu::separator {height: 1px; margin-left: 6px; margin-right: 6px; background: rgba(155, 155, 155, 255);}"
                    );
        widget->setPalette(qApp->palette());
        return;
    }
    widget->setStyleSheet(
                "QWidget{background-color: #191919; color: white;}"
                //reduce separator width
                "QWidget::separator {width: 1px} "
                //adjust menu style
                "QMenu::separator {height: 1px; margin-left: 6px; margin-right: 6px; background: rgba(155, 155, 155, 255);}"
                );

    //set style and color palette
    widget->setStyle(QStyleFactory::create("Fusion"));
    QPalette dark;
    dark.setColor(QPalette::Text, QColor(255, 255, 255));
    dark.setColor(QPalette::WindowText, QColor(255, 255, 255));
    dark.setColor(QPalette::Window, QColor(50, 50, 50));
    dark.setColor(QPalette::Button, QColor(50, 50, 50));
    dark.setColor(QPalette::Base, QColor(25, 25, 25));
    dark.setColor(QPalette::AlternateBase, QColor(50, 50, 50));
    dark.setColor(QPalette::ToolTipBase, QColor(200, 200, 200));
    dark.setColor(QPalette::ToolTipText, QColor(50, 50, 50));
    dark.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    dark.setColor(QPalette::BrightText, QColor(255, 50, 50));
    dark.setColor(QPalette::Link, QColor(40, 130, 220));
    dark.setColor(QPalette::Disabled, QPalette::Text, QColor(99, 99, 99));
    dark.setColor(QPalette::Disabled, QPalette::WindowText, QColor(99, 99, 99));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(99, 99, 99));
    dark.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    dark.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(99, 99, 99));
    widget->setPalette(dark);

    //adjust font size
    QFont font;
    font.setFamily(font.defaultFamily());
    font.setPointSize(8);
    widget->setFont(font);
}

void OCTproZ::slot_start() {
    if (this->currSystem == nullptr) {
        emit error(tr("Please chose a system!"));
        return;
    }

    //under certain circumstances, the OpenGL windows remain black. this fixes this issue
    this->resize(static_cast<float>(this->size().width()-1),static_cast<float>(this->size().height()-1));
    this->resize(static_cast<float>(this->size().width()+1),static_cast<float>(this->size().height()+1));
    this->forceUpdateProcessingParams();
    //update OpenGL texture size (this is only necessary in if test volume is activated, since the test volume changes the opengl texture size) //todo: emit signal for opengl texture size update only if test volume is active
    qDebug() << "slot_start:" << this->octParams->ascansPerBscan << ";" <<  this->octParams->bscansPerBuffer*this->octParams->buffersPerVolume << ";" << this->octParams->samplesPerLine/2;
    emit glBufferTextureSizeBscan(this->octParams->samplesPerLine/2, this->octParams->ascansPerBscan, this->octParams->bscansPerBuffer*this->octParams->buffersPerVolume);
    qApp->processEvents();
    //    emit glBufferTextureSizeEnFaceView(this->octParams->ascansPerBscan, this->octParams->bscansPerBuffer*this->octParams->buffersPerVolume, this->octParams->samplesPerLine/2);
    //    qApp->processEvents();
    //(re-)init resampling curve, dispersion curve, window curve, streaming //todo: carefully check if this is really necessary here

    //save current parameters to hdd
    this->saveSettings();

    //disable start/stop buttons
    this->actionStart->setEnabled(false);
    this->actionStop->setEnabled(false); //stop button will be enabled again as soon as processing initialization is done

    //enalbe 1d plotting and extensions
    emit allowRawGrabbing(true);

    //emit start signal to activate acquisition of current AcquisitionSystem
    emit start();

    //for debugging purposes: read out thread affinity of current thread
    qDebug() << "Main Thread ID start emit: " << QThread::currentThreadId();
}

void OCTproZ::slot_stop() {
    //adjust start stop buttons
    this->actionStart->setEnabled(true);
    this->actionStop->setEnabled(false);

    //disable 1d plotting and acquisition data grabbing by extensions
    emit allowRawGrabbing(false);
    QApplication::processEvents();

    //emit stop signal to stop acquisition system if it is still running
    if (this->currSystem != nullptr) {
        if(this->currSystem->acquisitionRunning){
            emit stop();
            QApplication::processEvents();
            this->currSystem->acquisitionRunning = false; //todo: think about whether OCTproZ should really set the AcquisitionRunning flag to false here, or whether only the acquisition system itself should be responsible for setting this flag to false
        }
    }
}

void OCTproZ::slot_record() {
    //check if system is open
    if (this->currSystem == nullptr) {
        emit error(tr("Nothing to record, no system opened."));
        return;
    }

    //check if the user selected anything to record
    RecordingParams recParams = this->octParams->recParams;
    if (!recParams.recordScreenshot && !recParams.recordRaw && !recParams.recordProcessed) {
        emit error(tr("Nothing to record! Please select what to record in the recording settings!"));
        return;
    }

    //save current parameters to hdd
    this->saveSettings();

    //set time stamp so it can be used in all file names of the same recording session
    recParams.timestamp = Settings::getInstance()->getTimestamp();

    //get user defined rec name
    QString recName = recParams.fileName;
    if (recName != "") {
        recName = "_" + recName;
    }

    //enable raw and processed recording
    if (recParams.recordProcessed) {
        this->slot_prepareGpu2HostForProcessedRecording();
    }
    if (recParams.recordRaw || recParams.recordProcessed) {
        this->sidebar->enableRecordTab(false);
        emit this->enableRecording(recParams);
        if (!this->currSystem->acquisitionRunning) {
            this->slot_start();
        }
    }

    //record screenshots
    if (recParams.recordScreenshot) {
        QString savePath = recParams.savePath;
        QString fileName = recParams.timestamp + recName + "_";
        if(this->bscanWindow->isVisible()) {
            this->bscanWindow->saveScreenshot(savePath, fileName + "bscan_snapshot.png");
        }
        if(this->enFaceViewWindow->isVisible()) {
            this->enFaceViewWindow->saveScreenshot(savePath, fileName + "enfaceview_snapshot.png");
        }
        if(this->volumeWindow->isVisible()) {
            this->volumeWindow->saveScreenshot(savePath, fileName + "volume_snapshot.png");
        }
    }

    //check if meta information should be saved
    if (recParams.saveMetaData) {
        QString metaFileName = recParams.savePath + "/" + recParams.timestamp + recName + "_meta.txt";
        Settings::getInstance()->copySettingsFile(metaFileName);
    }

}

void OCTproZ::slot_selectSystem() {
    QString selectedSystem = this->sysChooser->selectSystem(CCSystemManager::instance()->getSystemNames(true));
    if(selectedSystem == ""){
        this->error("Not Chose a System");
        return;
    }
    this->setSystem(selectedSystem);
}

void OCTproZ::slot_menuApplicationSettings() {
    //todo: application settings dialog
}

void OCTproZ::slot_menuSystemSettings() {
    if (this->currSystem != nullptr) {
        emit closeDock2D(); //GL window needs to be closed to avoid linux bug where QFileDialog is not usable when GL window is opend in background
        emit pluginSettingsRequest();
    }else{
        emit error(tr("No system opened!"));
    }
}

void OCTproZ::slot_menuExtensions() {
    //todo: refactor this method. Think of a better way to asociate corresponding extensions and qactions. Maybe store extensions and qactins in a qmap or in a qlist with qpairs.
    QAction* currAction = qobject_cast<QAction*>(sender());
    if(currAction == 0){return;}
    QString extensionName = currAction->text();
    Extension* extension = this->extManager->getExtensionByName(extensionName); //this just works if extension names are unique
    extension->settingsLoaded(Settings::getInstance()->getStoredSettings(extensionName)); //todo: use only signal slot to interact with extension (do not call methods directly like in this line) and move extensions to threads. Similar to AcquisitionSystems, see: implementation of activateSystem(AcquisitionSystem* system)
    if(extension == nullptr){
        emit error(tr("No Extension with name ") + extensionName + tr(" exists."));
        return;
    }
    QWidget* extensionWidget = extension->getWidget();
    QTabWidget* tabWidget = this->sidebar->getUi().tabWidget;

    //if extension is deactivated (i. e. not visible as tab within sidebar and not visible as separate window) and user checked the extension in the menu then activate it.
    if ((extension->getDisplayStyle() == SIDEBAR_TAB && tabWidget->indexOf(extensionWidget) == -1) || (extension->getDisplayStyle() == SEPARATE_WINDOW && !extensionWidget->isVisible())) {
        if(currAction->isChecked()){
            if(extension->getDisplayStyle() == SIDEBAR_TAB){
                tabWidget->addTab(extensionWidget, extensionName);
            } else if( extension->getDisplayStyle() == SEPARATE_WINDOW){
                extensionWidget->setWindowFlag(Qt::WindowStaysOnTopHint);
                extensionWidget->show();
            }
            connect(extension, &Extension::info, this->console, &MessageConsole::displayInfo);
            connect(extension, &Extension::error, this->console, &MessageConsole::displayError);
            connect(extension, &Extension::storeSettings, CCSignalBridge::instance(), &CCSignalBridge::slot_storePluginSettings);
            extension->activateExtension(); //todo: do not call extension methods directly, use signal slot (or invokeMethod -> see below) and run extension in separate thread
            //QMetaObject::invokeMethod(extension, "activateExtension", Qt::QueuedConnection); //todo: move activateExtension method to "slots" in extensions.h in devkit! move extension in separate thread
            connect(this, &OCTproZ::allowRawGrabbing, extension, &Extension::enableRawDataGrabbing);
            connect(this->signalProcessing, &Processing::streamingBufferEnabled, extension, &Extension::enableProcessedDataGrabbing);
            connect(this->processedDataNotifier, &Gpu2HostNotifier::newGpuDataAvailible, extension, &Extension::processedDataReceived);
            connect(this->signalProcessing, &Processing::rawData, extension, &Extension::rawDataReceived);
        }
    }
    //else (i.e. extension is visible within sidebar or as separate window) deactivate extension if user unchecked extension in menu
    else {
        if(!currAction->isChecked()) {
            if(extension->getDisplayStyle() == SIDEBAR_TAB){
                int index = tabWidget->indexOf(extensionWidget);
                tabWidget->removeTab(index);

                extension->deactivateExtension();
                disconnect(extension, &Extension::info, this->console, &MessageConsole::displayInfo);
                disconnect(extension, &Extension::error, this->console, &MessageConsole::displayError);
                disconnect(extension, &Extension::storeSettings, CCSignalBridge::instance(), &CCSignalBridge::slot_storePluginSettings);
                disconnect(this, &OCTproZ::allowRawGrabbing, extension, &Extension::enableRawDataGrabbing);
                disconnect(this->signalProcessing, &Processing::streamingBufferEnabled, extension, &Extension::enableProcessedDataGrabbing);
                disconnect(this->processedDataNotifier, &Gpu2HostNotifier::newGpuDataAvailible, extension, &Extension::processedDataReceived);
                disconnect(this->signalProcessing, &Processing::rawData, extension, &Extension::rawDataReceived);
            } else if( extension->getDisplayStyle() == SEPARATE_WINDOW){
                extensionWidget->close();
            }
        }
    }
}

void OCTproZ::slot_uncheckExtensionInMenu(Extension* extension) {
    //get corresponding action to closed extension
    QString extensionName = extension->getName();
    QAction* currAction = nullptr;
    foreach(auto action, this->extensionActions) {
        if(action->text() == extensionName){
            currAction = action;
            break;
        }
    }

    //uncheck action in menu
    currAction->setChecked(false);

    //disconnect signal slots from closed extension
    extension->deactivateExtension();
    disconnect(extension, &Extension::info, this->console, &MessageConsole::displayInfo);
    disconnect(extension, &Extension::error, this->console, &MessageConsole::displayError);
    disconnect(extension, &Extension::storeSettings, CCSignalBridge::instance(), &CCSignalBridge::slot_storePluginSettings);
    disconnect(this, &OCTproZ::allowRawGrabbing, extension, &Extension::enableRawDataGrabbing);
    disconnect(this->signalProcessing, &Processing::streamingBufferEnabled, extension, &Extension::enableProcessedDataGrabbing);
    disconnect(this->processedDataNotifier, &Gpu2HostNotifier::newGpuDataAvailible, extension, &Extension::processedDataReceived);
    disconnect(this->signalProcessing, &Processing::rawData, extension, &Extension::rawDataReceived);
}

void OCTproZ::slot_enableStopAction() {
    this->actionStop->setEnabled(true);
}

void OCTproZ::slot_updateAcquistionParameter(AcquisitionParams newParams){

    emit glBufferTextureSizeBscan(newParams.samplesPerAline/2, newParams.alinesPerBscan, newParams.bscansPerBuffer*newParams.buffersPerVolume);
    //emit glBufferTextureSizeEnFaceView(newParams.bscansPerBuffer, newParams.ascansPerBscan, newParams.samplesPerLine/2);
    emit glBufferTextureSizeEnFaceView(newParams.alinesPerBscan, newParams.bscansPerBuffer*newParams.buffersPerVolume, newParams.samplesPerAline/2);
    emit linesPerBufferChanged(newParams.alinesPerBscan * newParams.bscansPerBuffer);
    this->octParams->samplesPerLine = newParams.samplesPerAline;
    this->octParams->ascansPerBscan = newParams.alinesPerBscan;
    this->octParams->bscansPerBuffer = newParams.bscansPerBuffer;
    this->octParams->buffersPerVolume = newParams.buffersPerVolume;
    this->octParams->bitDepth = newParams.bitDepth;

    this->bscanWindow->setScanRange(newParams.lengthX, 4.61, true);
    this->enFaceViewWindow->setScanRange(newParams.lengthX, newParams.lengthY, false);

    CCSystemSettings settings(newParams.procFilePath);
    this->sidebar->loadSettingsFromSettings(&settings, false);

    this->sidebar->slot_setMaximumBscansForNoiseDetermination(this->octParams->bscansPerBuffer);

    this->forceUpdateProcessingParams();
}

void OCTproZ::slot_closeOpenGLwindows() {
#if !defined(Q_OS_WIN)
    if (this->dock2D->isVisible()) {
        this->isDock2DClosed = true;
        this->dock2D->setVisible(false);
    }
    if (this->dockEnFaceView->isVisible()) {
        this->isDockEnFaceViewClosed = true;
        this->dockEnFaceView->setVisible(false);
    }
    if (this->dockVolumeView->isVisible()) {
        this->isDockVolumeViewClosed = true;
        this->dockVolumeView->setVisible(false);
    }
#endif
}

void OCTproZ::slot_reopenOpenGLwindows() {
#if !defined(Q_OS_WIN)
    if (!this->dock2D->isVisible() && this->isDock2DClosed) {
        this->isDock2DClosed = false;
        this->dock2D->setVisible(true);
        this->dock2D->setFocus();
    }
    if (!this->dockEnFaceView->isVisible() && this->isDockEnFaceViewClosed) {
        this->isDockEnFaceViewClosed = false;
        this->dockEnFaceView->setVisible(true);
        this->dockEnFaceView->setFocus();
    }
    if (!this->dockVolumeView->isVisible() && this->isDockVolumeViewClosed) {
        this->isDockVolumeViewClosed = false;
        this->dockVolumeView->setVisible(true);
        this->dockVolumeView->setFocus();
    }
#endif
}

void OCTproZ::slot_prepareGpu2HostForProcessedRecording() {
    this->streamToHostMemorized = this->octParams->streamToHost;
    this->streamingBuffersToSkipMemorized = this->octParams->streamingBuffersToSkip;

    this->sidebar->getUi().spinBox_streamingBuffersToSkip->setValue(0);
    this->sidebar->getUi().groupBox_streaming->setChecked(true);
    this->sidebar->getUi().groupBox_streaming->setEnabled(false);
}

void OCTproZ::slot_resetGpu2HostSettings() {
    this->sidebar->getUi().spinBox_streamingBuffersToSkip->setValue(this->streamingBuffersToSkipMemorized);
    this->sidebar->getUi().groupBox_streaming->setChecked(this->streamToHostMemorized);
    this->sidebar->getUi().groupBox_streaming->setEnabled(true);
}

void OCTproZ::slot_recordingDone() {
    this->sidebar->enableRecordTab(true);
    if(this->octParams->recParams.stopAfterRecord && this->currSystem->acquisitionRunning){
        this->slot_stop();
    }
}

void OCTproZ::slot_enableEnFaceViewProcessing(bool enable) {
    this->octParams->enFaceViewEnabled = enable;
}

void OCTproZ::slot_enableBscanViewProcessing(bool enable) {
    this->octParams->bscanViewEnabled = enable;
}

void OCTproZ::slot_enableVolumeViewProcessing(bool enable) {
    this->octParams->volumeViewEnabled = enable;
    QCoreApplication::processEvents();
    if(enable){
        this->volumeWindow->update();
    }
}

void OCTproZ::slot_easterEgg() {
    if(this->dockVolumeView->isVisible()){
        if(this->currSystem == nullptr){
            this->volumeWindow->generateTestVolume();
        }
        if(this->currSystem != nullptr){
            if(!this->currSystem->acquisitionRunning){
                this->volumeWindow->generateTestVolume();
            }
        }
    }
}

void OCTproZ::slot_useCustomResamplingCurve(bool use) {
    this->octParams->useCustomResampleCurve = use;
    this->octParams->acquisitionParamsChanged = true;
    this->sidebar->slot_updateProcessingParams();
    this->sidebar->disableKlinCoeffInput(use);
}

void OCTproZ::slot_loadCustomResamplingCurve() {
    QString filters("CSV (*.csv)");
    QString defaultFilter("CSV (*.csv)");
    this->slot_closeOpenGLwindows();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Curve"), QDir::currentPath(), filters, &defaultFilter);
    this->slot_reopenOpenGLwindows();
    if(fileName == ""){
        emit error(tr("Loading of custom resampling curve for k-linearization canceled."));
        return;
    }

    this->loadResamplingCurveFromFile(fileName);
}

void OCTproZ::setSystem(QString systemName) {
    if(this->currSystemName == systemName){ //system already activated
        emit info(tr("System is already open."));
        return;
    }
    // INFO公用系统
    AcquisitionSystem* system = CCSystemManager::instance()->getSystemByName(systemName, true);

    if(system == nullptr){
        emit error(tr("Opening of OCT system failed. Could not find a system with the name: ") + systemName);
        return;
    }
    if(this->currSystem != nullptr){
        this->deactivateSystem(this->currSystem);
    }
    if(!this->activatedSystems.contains(systemName)){ //system got selected for the first time
        this->activatedSystems.append(systemName);
        this->activateSystem(system);
    }else{  //system was once active and needs to be reactivated now
        this->reactivateSystem(system);
    }
    this->currSystem = system;
    this->currSystemName = systemName;
    this->setWindowTitle(systemName);
    emit loadPluginSettings(Settings::getInstance()->getStoredSettings(systemName));
    this->actionStart->setEnabled(true);
    this->actionRecord->setEnabled(true);
    emit info(tr("System opened: ") + this->currSystemName);
    setStyleFormat(this->currSystem->settingsDialog);
    this->octParams->largeSystem = systemName.contains("Large");
}

void OCTproZ::activateSystem(AcquisitionSystem* system) {
    if(system != nullptr){
        if(this->currSystem != system){
            system->moveToThread(&acquisitionThread);
            connect(this, &OCTproZ::start, system, &AcquisitionSystem::startAcquisition);
            connect(this, &OCTproZ::stop, system, &AcquisitionSystem::stopAcquisition);
            connect(this, &OCTproZ::loadPluginSettings, system, &AcquisitionSystem::settingsLoaded);
            connect(system, &AcquisitionSystem::storeSettings, CCSignalBridge::instance(), &CCSignalBridge::slot_storePluginSettings);
            connect(system, &AcquisitionSystem::acquisitionStarted, this->signalProcessing, &Processing::slot_start);
            connect(system, &AcquisitionSystem::acquisitionStopped, this, &OCTproZ::slot_stop);
            connect(system->params, &AcquisitionParameter::updated, this, &OCTproZ::slot_updateAcquistionParameter);
            connect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::show);
            connect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::raise);
            connect(system->settingsDialog, &QDialog::finished, this, &OCTproZ::slot_reopenOpenGLwindows); //GL window needs to be closed to avoid linux bug where QFileDialog is not usable when GL window is opend in background
            connect(system, &AcquisitionSystem::info, this->console, &MessageConsole::displayInfo);
            connect(system, &AcquisitionSystem::error, this->console, &MessageConsole::displayError);
            connect(qApp, &QCoreApplication::aboutToQuit, system, &QObject::deleteLater);
            connect(system->buffer, &AcquisitionBuffer::info, this->console, &MessageConsole::displayInfo);
            connect(system->buffer, &AcquisitionBuffer::error, this->console, &MessageConsole::displayError);
            emit newSystem(system);
            acquisitionThread.start();
        }
    }
}

void OCTproZ::deactivateSystem(AcquisitionSystem* system) {
    this->slot_stop();
    QCoreApplication::processEvents(); //process events to ensure that acquisition is not running
    disconnect(this, &OCTproZ::start, system, &AcquisitionSystem::startAcquisition);
    disconnect(this, &OCTproZ::stop, system, &AcquisitionSystem::stopAcquisition);
    disconnect(this, &OCTproZ::loadPluginSettings, system, &AcquisitionSystem::settingsLoaded);
    disconnect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::show);
    disconnect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::raise);
}

void OCTproZ::reactivateSystem(AcquisitionSystem* system){
    connect(this, &OCTproZ::start, system, &AcquisitionSystem::startAcquisition);
    connect(this, &OCTproZ::stop, system, &AcquisitionSystem::stopAcquisition);
    connect(this, &OCTproZ::loadPluginSettings, system, &AcquisitionSystem::settingsLoaded);
    connect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::show);
    connect(this, &OCTproZ::pluginSettingsRequest, system->settingsDialog, &QDialog::raise);
}

void OCTproZ::forceUpdateProcessingParams() {
    this->octParams->acquisitionParamsChanged = true;
    this->sidebar->slot_updateProcessingParams();
}

void OCTproZ::updateSettingsMap() {
    //acquisition system
    this->mainWindowSettings.insert(MAIN_ACTIVE_SYSTEM, this->currSystemName);
    // selected folder
    this->mainWindowSettings.insert(LAST_SELECT_FOLDER, OCTGlobal::lastSelectedFolder);
}

void OCTproZ::loadResamplingCurveFromFile(QString fileName){
    if(fileName == ""){
        return;
    }
    QFile file(fileName);
    QVector<float> curve;
    file.open(QIODevice::ReadOnly);
    QTextStream txtStream(&file);
    QString line = txtStream.readLine();
    while (!txtStream.atEnd()){
        line = txtStream.readLine();
        curve.append((line.section(";", 1, 1).toFloat()));
    }
    file.close();
    if(curve.size() > 0){
        this->octParams->loadCustomResampleCurve(curve.data(), curve.size());
        this->octParams->acquisitionParamsChanged = true;
        this->sidebar->slot_updateProcessingParams();
        this->actionUseCustomKLinCurve->setEnabled(true);
        this->actionUseCustomKLinCurve->setChecked(true);
        emit info(tr("Custom resampling curve loaded! File used: ") + fileName);
    }else{
        emit error(tr("Custom resampling curve has a size of 0. Check if .csv file with resampling curve is not empty has right format."));
    }
}

void OCTproZ::loadMainWindowSettings(){
    Settings* settingsManager = Settings::getInstance();

    //load setting maps
    this->mainWindowSettings = settingsManager->getStoredSettings(MAIN_WINDOW_SETTINGS_GROUP);

    //apply loaded settings
    //	this->restoreGeometry(this->mainWindowSettings.value(MAIN_GEOMETRY).toByteArray());
    //this->restoreState(this->mainWindowSettings.value(MAIN_STATE).toByteArray()); //this crashes the application under certain circumstances. todo: find a way to save the gui state
    QString loadedSystemName = this->mainWindowSettings.value(MAIN_ACTIVE_SYSTEM).toString();
    if(loadedSystemName != ""){
        //        this->setSystem(loadedSystemName);
    }

    MessageConsole::MessageParams consoleParams;
    this->console->setParams(consoleParams);
    //
    OCTGlobal::lastSelectedFolder = this->mainWindowSettings.value(LAST_SELECT_FOLDER).toString();
}

void OCTproZ::saveMainWindowSettings() {
    this->updateSettingsMap();
    Settings* settingsManager = Settings::getInstance();
    settingsManager->storeSettings(MAIN_WINDOW_SETTINGS_GROUP, this->mainWindowSettings);
}

void OCTproZ::loadSettings(){
    this->sidebar->loadSettings();

    //restore main window position and size (if application was already open once)
    this->loadMainWindowSettings();

    //restore B-scan window and EnFaceView window settings
    this->bscanWindow->setSettings(Settings::getInstance()->getStoredSettings(this->bscanWindow->getName()));
    this->enFaceViewWindow->setSettings(Settings::getInstance()->getStoredSettings(this->enFaceViewWindow->getName()));
    this->volumeWindow->setSettings(Settings::getInstance()->getStoredSettings(this->volumeWindow->getName()));
}

void OCTproZ::saveSettings() {
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    Settings::getInstance()->setTimestamp(timestamp);
    this->sidebar->saveSettings();
    this->bscanWindow->saveSettings();
    this->enFaceViewWindow->saveSettings();
    this->volumeWindow->saveSettings();
    this->saveMainWindowSettings();
}


