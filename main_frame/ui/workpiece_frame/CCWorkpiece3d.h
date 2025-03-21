#ifndef CCWORKPIECE3D_H
#define CCWORKPIECE3D_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QOffscreenSurface>

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include <functional>
#include <vector>

#include <QtMath>

#include "oct_algorithm/glfunctions/mesh.h"
#include "oct_algorithm/glfunctions/raycastvolume.h"
#include "oct_algorithm/oct_params/settings.h"
#include "oct_forms/src/controlpanel.h"
#include "oct_forms/src/trackball.h"

#ifndef DELAY_TIME_IN_ms
#define DELAY_TIME_IN_ms 80
#endif


class CCWorkpiece3d : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
    explicit CCWorkpiece3d(QWidget *parent = nullptr);
	~CCWorkpiece3d();
    // 设置参数
    void setSettings(QVariantMap settings);
    // 获取参数
    QVariantMap getSettings();
    QString getName();

	void setStepLength(const GLfloat stepLength);
	void setThreshold(const GLfloat threshold);
	void setDepthWeight(const GLfloat depthWeight);
	void setSmoothFactor(const GLint smoothFactor);
	void setAlphaExponent(const GLfloat alphaExponent);
	void enableShading(const GLboolean shadingEnabled);
	void setMode(const QString& mode);
	void setBackground(const QColor& colour);
	void setStretch(const qreal x, const qreal y, const qreal z);
	void setGammaCorrection(float gamma);
	void generateTestVolume();

	QVector<QString> getModes();

	QColor getBackground();
    GLuint getGlBuffer();

    void initSize(unsigned int width, unsigned int height, unsigned int depth);
signals:
    void registerBufferCudaGL(unsigned int bufferId, quint32 screenNum, bool is3d);
	void initCudaGl();

	void dialogAboutToOpen();
	void dialogClosed();
public slots:
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;

	void changeTextureSize(unsigned int width, unsigned int height, unsigned int depth);
	void createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
	void registerOpenGLBufferWithCuda();
	void updateDisplayParams(GLWindow3DParams params);
	void openLUTFromImage(QImage lut);
	void saveSettings();
	//void resizeNoiseTexture();

protected:

	/*!
	 * \brief Initialise OpenGL-related state.
	 */
	void initializeGL() override;

	/*!
	 * \brief Paint a frame on the canvas.
	 */
	void paintGL() override;

	/*!
	 * \brief This function is called whenever the widget has been resized to update anything that depends on the widget's size.
	 * \param width New width.
	 * \param height New height.
	 */
	void resizeGL(int width, int height) override;

	/*!
	 * \brief This event handler is used to display the control panel when the mouse hovers over the widget
	 * \param event Widget enter event
	 */
	void enterEvent(QEvent* event) override;

	/*!
	 * \brief This event handler is used to hide the control panel when the mouse does not hovers over the widget anymore
	 * \param event Widget enter event
	 */
	void leaveEvent(QEvent* event) override;

private:
	bool delayedUpdatingRunning;
	bool initialized;
	bool changeTextureSizeFlag;
	bool updateContinuously;
	GLWindow3DParams displayParams;

    unsigned int volumeWidth = 0;
    unsigned int volumeHeight = 0;
    unsigned int volumeDepth = 0;

	QPoint mousePos;
	QPointF viewPos;
	qreal fps;
	QElapsedTimer timer;
	int counter;

	ControlPanel3D* panel;
	QVBoxLayout* layout;

	QMatrix4x4 viewMatrix;
	QMatrix4x4 modelViewProjectionMatrix;
	QMatrix3x3 normalMatrix;

	const GLfloat fov = 50.0f;										  /*!< Vertical field of view. */
	const GLfloat focalLength = 1.0 / qTan(M_PI / 180.0 * this->fov / 2.0); /*!< Focal length. */
	GLfloat aspectRatio;												/*!< width / height */

	QVector2D viewportSize;
	QVector3D rayOrigin; /*!< Camera position in model space coordinates. */

	QVector3D lightPosition {1.0, 3.0, 3.0};	/*!< In camera coordinates. */
	QVector3D diffuseMaterial {1.0, 1.0, 1.0};  /*!< Material colour. */
	GLfloat stepLength;						 /*!< Step length for ray march. */
	GLfloat threshold;						  /*!< Isosurface intensity threshold. */
	QColor background;						  /*!< Viewport background colour. */

	GLfloat gamma = 2.2f; /*!< Gamma correction parameter. */
	GLfloat depthWeight;
	GLfloat alphaExponent;
	GLint smoothFactor;
	GLboolean shadingEnabled;
	GLboolean lutEnabled;

	RayCastVolume *raycastingVolume;

	std::map<QString, QOpenGLShaderProgram*> shaders;
    // 显示方法
	std::map<QString, std::function<void(void)>> modes;
	QString activeMode;

	TrackBall trackBall {};	   /*!< Trackball holding the model rotation. */
	TrackBall sceneTrackBall {}; /*!< Trackball holding the scene rotation. */

	GLint distExp = -500;

    quint32 m_currentScreenNum;

    unsigned int VBO, VAO, EBO;

	/*!
	 * \brief Width scaled by the pixel ratio (for HiDPI devices).
	 */
	GLuint scaledWidth();

	/*!
	 * \brief Height scaled by the pixel ratio (for HiDPI devices).
	 */
	GLuint scaledHeight();

	/*!
	 * \brief Perform raycasting.
	 */
	void raycasting(const QString& shader);

	/*!
	 * \brief Convert a mouse position into normalised canvas coordinates.
	 * \param p Mouse position.
	 * \return Normalised coordinates for the mouse position.
	 */
	QPointF pixelPosToViewPos(const QPointF& p);

	/*!
	 * \brief Add a shader.
	 * \param name Name for the shader.
	 * \param vertex Vertex shader source file.
	 * \param fragment Fragment shader source file.
	 */
	void addShader(const QString& name, const QString& vector, const QString& fragment);
	void restoreLUTSettingsFromPreviousSession();
	void delayedUpdate();
	void countFPS();
};

#endif //CCWORKPIECE3D_H
