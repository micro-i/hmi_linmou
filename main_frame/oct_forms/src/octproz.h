#ifndef OCTPROZ_H
#define OCTPROZ_H

#ifdef _WIN32
	#define WINDOWS_LEAN_AND_MEAN
	//#define NOMINMAX
    #include "GL/glew.h"
	#include <windows.h>
#endif

#include <QMainWindow>
#include <QThread>
#include <QStatusBar>
#include <QVariantMap>
#include <qdir.h>
#include <qpluginloader.h>
#include "sidebar.h"
#include "messageconsole.h"
#include "plotwindow1d.h"
#include "glwindow2d.h"
#include "glwindow3d.h"

#include "oct_algorithm/CCSystemManager.h"

#include "systemchooser.h"
#include "extensionmanager.h"
#include "extensioneventfilter.h"
#include "oct_algorithm/processing.h"
#include "oct_devkit/src/microi_devkit.h"
#include "oct_algorithm/oct_params/octalgorithmparameters.h"

#include "ui_octproz.h"

#define APP_VERSION "1.6.0"
#define APP_VERSION_DATE "24 June 2022"
#define APP_NAME "OCTproZ"

#define MAIN_WINDOW_SETTINGS_GROUP "main_window_settings"
#define MAIN_GEOMETRY "main_geometry"
#define MAIN_STATE "main_state"
#define MAIN_ACTIVE_SYSTEM "main_active_system"
#define MESSAGE_CONSOLE_BOTTOM "message_console_bottom"
#define LAST_SELECT_FOLDER "last_selected_folder"




namespace Ui {
class OCTproZ;
}

class OCTproZ : public QWidget
{
	Q_OBJECT
    QThread acquisitionThread;

public:
	explicit OCTproZ(QWidget* parent = 0);
	~OCTproZ();

	void closeEvent(QCloseEvent* event);

    void initProcessing();
    void deinitProcessing();
	void initMenu();
	void initGui();

	void prepareDockWidget(QDockWidget*& dock, QWidget* widgetForDock,  QAction*& action, const QIcon& icon, QString iconText);
	void initActionsAndDocks();
	void loadSystemsAndExtensions();
	void initExtensionsMenu();
    void updateSiderBar();
    void setStyleFormat(QWidget*, bool origin = false);
public slots:
	void slot_start();
	void slot_stop();
	void slot_record();
	void slot_selectSystem();
	void slot_menuApplicationSettings();
	void slot_menuSystemSettings();
	void slot_menuExtensions();
	void slot_uncheckExtensionInMenu(Extension* extension);
	void slot_enableStopAction();
	void slot_updateAcquistionParameter(AcquisitionParams newParams);
	void slot_closeOpenGLwindows();
	void slot_reopenOpenGLwindows();
	void slot_prepareGpu2HostForProcessedRecording();
	void slot_resetGpu2HostSettings();
	void slot_recordingDone();
	void slot_enableEnFaceViewProcessing(bool enable);
	void slot_enableBscanViewProcessing(bool enable);
	void slot_enableVolumeViewProcessing(bool enable);
	void slot_easterEgg();
	void slot_useCustomResamplingCurve(bool use);
	void slot_loadCustomResamplingCurve();


private:
	void setSystem(QString systemName);
	void activateSystem(AcquisitionSystem* system);
	void deactivateSystem(AcquisitionSystem* system);
	void reactivateSystem(AcquisitionSystem* system);
	void forceUpdateProcessingParams();
	void loadMainWindowSettings();
	void saveMainWindowSettings();
	void loadSettings();
	void saveSettings();
	void updateSettingsMap();
	void loadResamplingCurveFromFile(QString fileName);

	Ui::OCTproZ *ui;
    Sidebar* sidebar;

    OctAlgorithmParameters* octParams;

    SystemChooser* sysChooser;
    AcquisitionSystem* currSystem;
    QString currSystemName;
    MessageConsole* console;

    Processing* signalProcessing;

    QList<QString> activatedSystems;

    QList<QAction*> extensionActions;
    ExtensionManager* extManager;

    Gpu2HostNotifier* processedDataNotifier;

	QVariantMap mainWindowSettings;








	bool processingInThread;
	bool streamToHostMemorized;
	unsigned int streamingBuffersToSkipMemorized;

	QMenu* extrasMenu;

	QAction* actionStart;
	QAction* actionStop;
	QAction* actionRecord;
	QAction* actionSelectSystem;
	QAction* actionSystemSettings;
	QAction* action1D;
	QAction* action2D;
	QAction* actionEnFaceView;
	QAction* action3D;
	QAction* actionConsole;
	QAction* actionUseSidebarKLinCurve;
	QAction* actionUseCustomKLinCurve;
	QAction* actionSetCustomKLinCurve;




	QDockWidget* dockConsole;
	QDockWidget* dock1D;
	QDockWidget* dock2D;
	QDockWidget* dockEnFaceView;
	QDockWidget* dockVolumeView;

	GLWindow3D* volumeWindow;
	GLWindow2D* bscanWindow;
	GLWindow2D* enFaceViewWindow;
	PlotWindow1D* plot1D;
    QStatusBar* m_statusBar;
    QToolBar* controlToolBar;


	bool isDock2DClosed;
	bool isDockEnFaceViewClosed;
	bool isDockVolumeViewClosed;

	QOpenGLContext *context;

signals:
	void start();
	void stop();
	void allowRawGrabbing(bool allowed);
	void record();
	void enableRecording(RecordingParams recParams);
	void pluginSettingsRequest();
	void newSystemSelected();
	void newSystem(AcquisitionSystem*);
	void error(QString);
	void info(QString);
	void glBufferTextureSizeBscan(unsigned int width, unsigned int height, unsigned int depth);
	void glBufferTextureSizeEnFaceView(unsigned int width, unsigned int height, unsigned int depth);
	void linesPerBufferChanged(int linesPerBuffer);
	void closeDock2D();
	void reopenDock2D();
	void loadPluginSettings(QVariantMap);
    void registerBufferCudaGL(unsigned int bufferId);


};

#endif // OCTPROZ_H
