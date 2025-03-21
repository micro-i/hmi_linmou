#ifndef GLWINDOW2D_H
#define GLWINDOW2D_H

#define EXTENDED_PANEL "extended_panel"
#define DISPLAYED_FRAMES "displayed_frames"
#define CURRENT_FRAME "current_frame"
#define ROTATION_ANGLE "rotation_angle"
#define DISPLAY_MODE "display_mode"
#define STRETCH_X "stretch_x"
#define STRETCH_Y "stretch_y"
#define HORIZONTAL_SCALE_BAR_ENABLED "horizontal_scale_bar_enabeld"
#define VERTICAL_SCALE_BAR_ENABLED "vertical_scale_bar_enabeld"
#define HORIZONTAL_SCALE_BAR_TEXT "horizontal_scale_bar_text"
#define VERTICAL_SCALE_BAR_TEXT "vertical_scale_bar_text"
#define HORIZONTAL_SCALE_BAR_LENGTH "horizontal_scale_bar_length"
#define VERTICAL_SCALE_BAR_LENGTH "vertical_scale_bar_length"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QOffscreenSurface>

// CUDA Runtime, Interop  includes
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <cuda_profiler_api.h>
#include <vector_types.h>
#include <vector_functions.h>
#include <driver_functions.h>

// CUDA helper functions
#include <helper_cuda.h>
#include <helper_functions.h>
#include <helper_timer.h>

// Qt gui includes
#include <QSpinBox>
#include <QLabel>
#include <QLayout>
#include <QSlider>
#include <QMenu>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QToolButton>
#include <QPainter>
#include <QCheckBox>
#include <QLineEdit>

#include <atomic>

#include "stringspinbox.h"
#include "outputwindow.h"

//#include "octalgorithmparameters.h" //needed for the definition of DISPLAY_FUNCTION enum


#define DEFAULT_WIDTH  2048
#define DEFAULT_HEIGHT 2048

#define DELAY_TIME_IN_ms 40


struct LineCoordinates{
	float x1;
	float y1;
	float x2;
	float y2;
};

struct GLWindow2DParams {
	bool extendedViewEnabled;
	int displayedFrames;
	int currentFrame;
    double rotationAngle = 0.0;
	int displayFunction;
    double stretchX = 1.0;
    double stretchY = 1.0;
	bool horizontalScaleBarEnabled;
	bool verticalScaleBarEnabled;
	int horizontalScaleBarLength;
	int verticalScaleBarLength;
	QString horizontalScaleBarText;
	QString verticalScaleBarText;
};

enum FRAME_EDGE {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT
};


class ScaleBar;
class ControlPanel2D;
class GLWindow2D : public QOpenGLWidget, protected QOpenGLFunctions, public OutputWindow
{
	Q_OBJECT

public:
	GLWindow2D(QWidget* parent);
	~GLWindow2D();

	ControlPanel2D* getControlPanel()const {return this->panel;}
	FRAME_EDGE getMarkerOrigin() const {return this->markerOrigin;}
	void setMarkerOrigin(FRAME_EDGE origin);
	QAction* getMarkerAction(){return this->markerAction;}
    QAction* getCoordAction(){return this->coordAction;}
	void setSettings(QVariantMap settings) override;
	QVariantMap getSettings() override;

    void setScanRange(float screenx, float screeny, bool bscan);

private:
	unsigned int width;
	unsigned int height;
	unsigned int depth;

    float m_xRange = 0;
    float m_yRange = 0;
    bool m_isBscan = false;


	float xTranslation;
	float yTranslation;
	float scaleFactor;
	float screenWidthScaled;
	float screenHeightScaled;
	bool initialized;
	bool changeBufferSizeFlag;
	bool keepAspectRatio;
	bool delayedUpdatingRunning;
	float stretchX;
	float stretchY;
	float rotationAngle;
	//OctAlgorithmParameters::DISPLAY_FUNCTION displayFuntion;
	unsigned int markerPosition;
	bool markerEnabled;
    bool coordinateEnabled;

	QMenu* contextMenu;
	QAction* keepAspectRatioAction;
	QAction* markerAction;
    QAction* coordAction;
	QAction* screenshotAction;

	FRAME_EDGE markerOrigin;
	LineCoordinates markerCoordinates;

	GLuint buf;
	GLuint texture;

	QPoint mousePos;

	size_t frameNr;

	ControlPanel2D* panel;
	QVBoxLayout* layout;

	ScaleBar* horizontalScaleBar;
	ScaleBar* verticalScaleBar;

	void initContextMenu();
	void displayScalebars();
	void displayMarker();
	void displayOrientationLine(int x, int y, int length);
	void delayedUpdate();

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;
    inline void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
    inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                                        const GLdouble model[16], const GLdouble proj[16],
    const GLint viewport[4],
    GLdouble * winx, GLdouble * winy, GLdouble * winz);
    void renderText(double x, double y, double z, const QString &text, const QFont& font);
public slots:
	void saveScreenshot(QString savePath, QString fileName);
	void changeTextureSize(unsigned int width, unsigned int height, unsigned int depth);
	void createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
	void registerOpenGLBufferWithCuda();
	void setKeepAspectRatio(bool enable);
	void setRotationAngle(float angle);
	void setStretchX(float stretchFactor);
	void setStretchY(float stretchFactor);
	void openScreenshotDialog();
	void enableMarker(bool enable);
    void enableCoordinate(bool enable);
	void setMarkerPosition(unsigned int position);
	void saveSettings();


signals:
	void registerBufferCudaGL(unsigned int bufferId);
	void initCudaGl();
	void currentFrameNr(unsigned int frameNr);

	void dialogAboutToOpen();
	void dialogClosed();
	void error(QString);
	void info(QString);

};






class ScaleBar : public QObject, public QPainter
{
	Q_OBJECT
public:
	enum ScaleBarOrientation {
		Horizontal,
		Vertical
	};

	ScaleBar();
	~ScaleBar();

	void enable(bool enable){this->enabled = enable;}
	bool isEnabled(){return this->enabled;}
	void setOrientation(ScaleBarOrientation orientation){this->orientation = orientation;}
	void setPos(QPoint pos){this->pos = pos;}
	void setLength(int lengthInPx){this->lenghInPx = lengthInPx;}
	void setText(QString text);
	void draw(QPaintDevice* paintDevice, float scaleFactor = 1.0);


private:
	void updateTextSizeInfo();

	bool enabled;
	ScaleBarOrientation orientation;
	QPoint pos;
	int lenghInPx;
	QString text;
	int textDistanceToScaleBarInPx;
	int textWidth;
	int textHeight;
	bool textChanged;
};





class ControlPanel2D : public QWidget
{
	Q_OBJECT

public:
	ControlPanel2D(QWidget* parent);
	~ControlPanel2D();

	void setMaxFrame(unsigned int maxFrame);
	void setMaxAverage(unsigned int maxAverage);
	GLWindow2DParams getParams();


private:
	void findGuiElements();
	void enableExtendedView(bool enable);
	void updateParams();
	void connectGuiToSettingsChangedSignal();
	void disconnectGuiFromSettingsChangedSignal();

	GLWindow2DParams params;
	bool extendedView;
	QWidget* panel;
	QSpinBox* spinBoxAverage;
	QSpinBox* spinBoxFrame;
	QDoubleSpinBox* doubleSpinBoxRotationAngle;
	QLabel* labelFrame;
	QLabel* labelRotationAngle;
	QLabel* labelDisplayFunctionFrames;
	QLabel* labelDisplayFunction;
	QSlider* slider;
	//QComboBox* comboBox; //due to a Qt bug QComboBox is not displayed correctly on top of a QOpenGLWidget. StringSpinBox is used as work around
	StringSpinBox* stringBoxFunctions;
	QToolButton* toolButtonMore;
	QDoubleSpinBox* doubleSpinBoxStretchX;
	QLabel* labelStretchX;
	QDoubleSpinBox* doubleSpinBoxStretchY;
	QLabel* labelStretchY;
	QCheckBox* checkBoxHorizontalScaleBar;
	QLabel* labelHorizontalScaleBar;
	QSpinBox* spinBoxHorizontalScaleBar;
	QLabel* labelHorizontalScaleBarText;
	QLineEdit* lineEditHorizontalScaleBarText;
	QCheckBox* checkBoxVerticalScaleBar;
	QLabel* labelVerticalScaleBar;
	QLabel* labelVerticalScaleBarText;
	QSpinBox* spinBoxVerticalScaleBar;
	QLineEdit* lineEditVerticalScaleBarText;

	QHBoxLayout* widgetLayout;
	QGridLayout* layout;

	QList<QLineEdit*> lineEdits;
	QList<QCheckBox*> checkBoxes;
	QList<QDoubleSpinBox*> doubleSpinBoxes;
	QList<QSpinBox*> spinBoxes;
	QList<StringSpinBox*> stringSpinBoxes;
	QList<QComboBox*> comboBoxes;


protected:


public slots:
	void setParams(GLWindow2DParams params);
	void updateDisplayFrameSettings();
	void toggleExtendedView();


signals:
	void displayFrameSettingsChanged(unsigned int frameNr, unsigned int displayFunctionFrames, int displayFunction);
	void settingsChanged();


friend class GLWindow2D;
};

#endif // GLWINDOW2D_H
