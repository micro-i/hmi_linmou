#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include <QtWidgets>
#include <QDebug>

#include "CCWorkpiece3d.h"
#include "infrastructure/OCTGlobal.h"



inline QVector3D toVector3D(const QColor& colour) {
    return QVector3D(colour.redF(), colour.greenF(), colour.blueF());
}

CCWorkpiece3d::CCWorkpiece3d(QWidget *parent) : QOpenGLWidget {parent}, raycastingVolume {nullptr}
{
    qRegisterMetaType<GLWindow3DParams>("GLWindow3DParams");
    qRegisterMetaType<quint32>("quint32");
    // Register available rendering modes here
    this->panel = new ControlPanel3D(this);
    // TODO 最大宽度
    this->panel->setMaximumWidth(250);
    this->panel->setStyleSheet("QWidget{background-color: rgba(0, 0, 0, 0); color: rgba(255, 255, 255, 1);}"
                               "QAbstractSpinBox{background-color: rgba(39, 42, 51, 1); color: rgba(255, 255, 255, 1);"
                               "}");
    QStringList modes = { "Isosurface", "MIDA", "Alpha blending", "X-ray", "DMIP", "MIP"};
    // 显示模式
    for(auto& mode : modes){
        this->modes[mode] = [=](){CCWorkpiece3d::raycasting(mode);};
    }
    this->panel->setModes(modes);
    // 参数初始化
    this->delayedUpdatingRunning = false;
    this->initialized = false;
    this->changeTextureSizeFlag = false;
    this->updateContinuously = false;
    this->raycastingVolume = nullptr;
    //init panel
    this->layout = new QVBoxLayout(this);
    this->layout->addStretch();
    this->layout->addWidget(this->panel);
    this->panel->setVisible(false);

    this->setMode("MIP");
    this->setThreshold(30);
    this->setStepLength(0.01f);

    this->fps = 0.0;
    //this->timer.start();
    this->counter = 0;

    this->viewPos.setX(0);
    this->viewPos.setY(0);

    connect(this->panel, &ControlPanel3D::displayParametersChanged, this, &CCWorkpiece3d::updateDisplayParams);
    connect(this->panel, &ControlPanel3D::lutSelected, this, &CCWorkpiece3d::openLUTFromImage);
    connect(this->panel, &ControlPanel3D::dialogAboutToOpen, this, &CCWorkpiece3d::dialogAboutToOpen);
    connect(this->panel, &ControlPanel3D::dialogClosed, this, &CCWorkpiece3d::dialogClosed);

    this->setSettings(Settings::getInstance()->getStoredSettings("3d-volume-window"));

    setBackground(QColor(82, 87, 110, 1));
}


CCWorkpiece3d::~CCWorkpiece3d()
{
    this->saveSettings();
    foreach (auto element, this->shaders) {
        delete element.second;
    }
    delete this->raycastingVolume;
}

void CCWorkpiece3d::setSettings(QVariantMap settings) {
    GLWindow3DParams params;
    //    params.extendedViewEnabled = settings.value(EXTENDED_PANEL).toBool();
    params.extendedViewEnabled = false;
    params.displayMode = settings.value(DISPLAY_MODE).toString();
    params.displayModeIndex = settings.value(DISPLAY_MODE_INDEX).toInt();
    params.threshold = settings.value(THRESHOLD).toReal();
    params.rayMarchStepLength = settings.value(RAY_STEP_LENGTH).toReal();
    params.stretchX = settings.value(STRETCH_X).toReal();
    params.stretchY= settings.value(STRETCH_Y).toReal();
    params.stretchZ = settings.value(STRETCH_Z).toReal();
    params.gamma = settings.value(GAMMA).toReal();
    params.depthWeight = settings.value(DEPTH_WEIGHT).toReal();
    params.smoothFactor = settings.value(SMOOTH_FACTOR).toInt();
    params.alphaExponent = settings.value(ALPHA_EXPONENT).toReal();
    params.shading = settings.value(SHADING_ENABLED).toBool();
    params.lutEnabled = settings.value(LUT_ENABLED).toBool();
    params.lutFileName = settings.value(LUT_FILENAME).toString();
    this->panel->setParams(params);
}

QVariantMap CCWorkpiece3d::getSettings() {
    QVariantMap settings;
    GLWindow3DParams params = this->panel->getParams();
    settings.insert(EXTENDED_PANEL, params.extendedViewEnabled);
    settings.insert(DISPLAY_MODE, params.displayMode);
    settings.insert(DISPLAY_MODE_INDEX, params.displayModeIndex);
    settings.insert(THRESHOLD, params.threshold);
    settings.insert(RAY_STEP_LENGTH, params.rayMarchStepLength);
    settings.insert(STRETCH_X, params.stretchX);
    settings.insert(STRETCH_Y, params.stretchY);
    settings.insert(STRETCH_Z, params.stretchZ);
    settings.insert(GAMMA, params.gamma);
    settings.insert(DEPTH_WEIGHT, params.depthWeight);
    settings.insert(SMOOTH_FACTOR, params.smoothFactor);
    settings.insert(ALPHA_EXPONENT, params.alphaExponent);
    settings.insert(SHADING_ENABLED, params.shading);
    settings.insert(LUT_ENABLED, params.lutEnabled);
    settings.insert(LUT_FILENAME, params.lutFileName);
    return settings;
}

QString CCWorkpiece3d::getName()
{
    return "CCWorkpiece3d";
}

void CCWorkpiece3d::setStepLength(const GLfloat step_length) {
    this->stepLength = step_length;
    update();
}

void CCWorkpiece3d::setThreshold(const GLfloat threshold) {
    this->threshold = threshold;
    update();
}

void CCWorkpiece3d::setDepthWeight(const GLfloat depth_weight) {
    this->depthWeight = depth_weight;
    update();
}

void CCWorkpiece3d::setSmoothFactor(const GLint smooth_factor) {
    this->smoothFactor = smooth_factor;
    update();
}

void CCWorkpiece3d::setAlphaExponent(const GLfloat alpha_exponent) {
    this->alphaExponent = alpha_exponent;
    update();
}

void CCWorkpiece3d::enableShading(const GLboolean shading_enabled) {
    this->shadingEnabled = shading_enabled;
    update();
}

void CCWorkpiece3d::setMode(const QString &mode) {
    this->activeMode = mode;
    update();
}

void CCWorkpiece3d::setBackground(const QColor &colour) {
    this->background = colour;
    update();
}

void CCWorkpiece3d::setStretch(const qreal x, const qreal y, const qreal z) {
    if(this->raycastingVolume != nullptr){
        this->raycastingVolume->setStretch(x, y, z);
        update();
    }
}

void CCWorkpiece3d::setGammaCorrection(float gamma) {
    this->gamma = gamma;
}

void CCWorkpiece3d::generateTestVolume() {
    if(this->raycastingVolume != nullptr){
        makeCurrent();
        this->raycastingVolume->generateTestVolume();
        doneCurrent();
    }
}

QVector<QString> CCWorkpiece3d::getModes() {
    QVector<QString> modes;
    foreach(auto element, this->modes){
        QString key = element.first;
        modes.append(key);
    }
    return modes;
}

QColor CCWorkpiece3d::getBackground() {
    return this->background;
}

GLuint CCWorkpiece3d::getGlBuffer()
{
    return this->raycastingVolume->getVolumeTexture();
}

void CCWorkpiece3d::initSize(unsigned int width, unsigned int height, unsigned int depth)
{
    this->volumeWidth = width;
    this->volumeHeight = height;
    this->volumeDepth = depth;
}

void CCWorkpiece3d::initializeGL() {
    initializeOpenGLFunctions();

    if(this->raycastingVolume != nullptr){
        delete this->raycastingVolume;
        this->raycastingVolume = nullptr;
    }
    this->raycastingVolume = new RayCastVolume();
    this->raycastingVolume->createNoise();

    if(!this->initialized){
        this->addShader("Isosurface", ":/shaders/isosurface.vert", ":/shaders/isosurface.frag");
        this->addShader("Alpha blending", ":/shaders/alpha_blending.vert", ":/shaders/alpha_blending.frag");
        this->addShader("MIP", ":/shaders/maximum_intensity_projection.vert", ":/shaders/maximum_intensity_projection.frag");
        this->addShader("DMIP", ":/shaders/depth_mip.vert", ":/shaders/depth_mip.frag");
        this->addShader("MIDA", ":/shaders/mida.vert", ":/shaders/mida.frag");
        this->addShader("X-ray", ":/shaders/xray.vert", ":/shaders/xray.frag");
    }

    if(this->initialized){
        emit registerBufferCudaGL(this->raycastingVolume->getVolumeTexture(), m_currentScreenNum, true); //registerBufferCudaGL is necessary here because as soon as the openglwidget/dock is removed from the main window initializeGL() is called again. //todo: check if opengl context (buffer, texture,...) cleanup is necessary!
    }
    this->initialized = true;

    if(this->changeTextureSizeFlag){
        this->changeTextureSize(this->volumeWidth, this->volumeHeight, this->volumeDepth);
        this->changeTextureSizeFlag = false;
    }

    this->updateDisplayParams(this->displayParams); //set display parameters that are restored from previous octproz session

    this->restoreLUTSettingsFromPreviousSession();

//    float vertices[] = {
//        0.5f,  0.5f, 0.0f,  // top right
//        0.5f, -0.5f, 0.0f,  // bottom right
//        -0.5f, -0.5f, 0.0f,  // bottom left
//        -0.5f,  0.5f, 0.0f   // top left
//    };
//    unsigned int indices[] = {  // note that we start from 0!
//                                0, 1, 3,  // first Triangle
//                                1, 2, 3   // second Triangle
//                             };
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);
//    glBindVertexArray(VAO);

//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
}

void CCWorkpiece3d::resizeGL(int w, int h) {
    this->viewportSize = {static_cast<float>(this->scaledWidth()), static_cast<float>(this->scaledHeight())};
    this->aspectRatio = static_cast<float>(this->scaledWidth()) / static_cast<float>(this->scaledHeight());
    glViewport(0, 0, this->scaledWidth(), this->scaledHeight());
    //this->raycastingVolume->createNoise();  //todo: improve noise data generation. for large display resolutions noise calculation takes so lang that it slows down the process of window resizing noticeably
}

void CCWorkpiece3d::paintGL() {
    //this->countFPS();

    // Compute geometry
    this->viewMatrix.setToIdentity();
    this->viewMatrix.translate(this->viewPos.x(), this->viewPos.y(), -4.0f * qExp(this->distExp / 600.0f));
    this->viewMatrix.rotate(this->trackBall.rotation());

    this->modelViewProjectionMatrix.setToIdentity();
    this->modelViewProjectionMatrix.perspective(this->fov, static_cast<float>(this->scaledWidth()) / static_cast<float>(this->scaledHeight()), 0.1f, 50.0f);
    this->modelViewProjectionMatrix *= this->viewMatrix * raycastingVolume->modelMatrix();

    this->normalMatrix = (this->viewMatrix * raycastingVolume->modelMatrix()).normalMatrix();

    this->rayOrigin = this->viewMatrix.inverted() * QVector3D({0.0f, 0.0f, 0.0f});

    // Perform raycasting
    if(this->modes.count(this->activeMode)){
        this->modes[this->activeMode]();
    }


    if(this->updateContinuously){
        update();
    }else{
        if(!this->delayedUpdatingRunning){
            this->delayedUpdatingRunning = true;
            QTimer::singleShot(DELAY_TIME_IN_ms, this, QOverload<>::of(&CCWorkpiece3d::delayedUpdate)); //todo: consider using Gpu2HostNotifier to notify GLWindow3D when new volume data is available
        }
    }
}

GLuint CCWorkpiece3d::scaledWidth() {
    return devicePixelRatio() * width();
}

GLuint CCWorkpiece3d::scaledHeight() {
    return devicePixelRatio() * height();
}

void CCWorkpiece3d::raycasting(const QString& shader) {
    this->shaders[shader]->bind();
    {
        this->shaders[shader]->setUniformValue("ViewMatrix", this->viewMatrix);
        this->shaders[shader]->setUniformValue("ModelViewProjectionMatrix", this->modelViewProjectionMatrix);
        this->shaders[shader]->setUniformValue("NormalMatrix", this->normalMatrix);
        this->shaders[shader]->setUniformValue("aspect_ratio", this->aspectRatio);
        this->shaders[shader]->setUniformValue("focal_length", this->focalLength);
        this->shaders[shader]->setUniformValue("viewport_size", this->viewportSize);
        this->shaders[shader]->setUniformValue("ray_origin", this->rayOrigin);
        this->shaders[shader]->setUniformValue("top", raycastingVolume->top());
        this->shaders[shader]->setUniformValue("bottom", raycastingVolume->bottom());
        this->shaders[shader]->setUniformValue("background_colour", toVector3D(this->background));
        this->shaders[shader]->setUniformValue("light_position", this->lightPosition);
        this->shaders[shader]->setUniformValue("material_colour", this->diffuseMaterial);
        this->shaders[shader]->setUniformValue("step_length", this->stepLength);
        this->shaders[shader]->setUniformValue("threshold", this->threshold);
        this->shaders[shader]->setUniformValue("gamma", this->gamma);
        this->shaders[shader]->setUniformValue("volume", 0);
        this->shaders[shader]->setUniformValue("jitter", 1);
        this->shaders[shader]->setUniformValue("lut", 2);
        this->shaders[shader]->setUniformValue("depth_weight", this->depthWeight);
        this->shaders[shader]->setUniformValue("smooth_factor", this->smoothFactor);
        this->shaders[shader]->setUniformValue("alpha_exponent", this->alphaExponent);
        this->shaders[shader]->setUniformValue("shading_enabled", this->shadingEnabled);
        this->shaders[shader]->setUniformValue("lut_enabled", this->lutEnabled);

        glClearColor(this->background.redF(), this->background.greenF(), this->background.blueF(), this->background.alphaF());
        glClear(GL_COLOR_BUFFER_BIT);

        raycastingVolume->paint();
    }
    this->shaders[shader]->release();

    //    // 绘制变换
    //    auto matrix = this->viewMatrix;
    //    matrix.scale(this->distExp / 600.0);
    //    QVector4D vertice1 = {-1.0, -1.0, 0.0, 1.0};
    //    QVector4D vertice2 = {-1.0, 1.0, 0.0, 1.0};
    //    QVector4D vertice3 = {1.0, 1.0, 0.0, 1.0};
    //    QVector4D vertice4 = {1.0, -1.0, 0.0, 1.0};
    //    vertice1 = matrix * vertice1;
    //    vertice2 = matrix * vertice2;
    //    vertice3 = matrix * vertice3;
    //    vertice4 = matrix * vertice4;
    //    glLineWidth(3.0);
    //    glBegin(GL_QUADS);
    //    glColor4ub(0, 200, 0, 30);
    //    glVertex3f(vertice1.x(), vertice1.y(), 0);
    //    glVertex3f(vertice2.x(), vertice2.y(), 0);
    //    glVertex3f(vertice3.x(), vertice3.y(), 0);
    //    glVertex3f(vertice4.x(), vertice4.y(), 0);
    //    glEnd();
}

QPointF CCWorkpiece3d::pixelPosToViewPos(const QPointF& p) {
    return QPointF(2.0 * static_cast<float>(p.x()) / width() - 1.0, 1.0 - 2.0 * static_cast<float>(p.y()) / height());
}

void CCWorkpiece3d::mouseDoubleClickEvent(QMouseEvent *event) {
    if(!this->panel->underMouse()){
        this->distExp = -500;
        this->viewPos.setX(0);
        this->viewPos.setY(0);
        update();
    }
}

void CCWorkpiece3d::enterEvent(QEvent *event) {
    this->panel->setVisible(true);
}


void CCWorkpiece3d::leaveEvent(QEvent *event) {
    this->panel->setVisible(false);
}

void CCWorkpiece3d::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !this->panel->underMouse()) {
        this->trackBall.move(pixelPosToViewPos(event->pos()), this->sceneTrackBall.rotation().conjugated());
    }else if(event->buttons() &Qt::MiddleButton && !this->panel->underMouse()){
        QPoint delta = (event->pos() - this->mousePos);
        int windowWidth = this->size().width();
        int windowHeight = this->size().height();
        this->viewPos.rx() += 2.0*static_cast<float>(delta.x())/static_cast<float>(windowWidth);
        this->viewPos.ry() += -2.0*static_cast<float>(delta.y())/static_cast<float>(windowHeight);
    } else {
        this->trackBall.release(pixelPosToViewPos(event->pos()), this->sceneTrackBall.rotation().conjugated());
    }
    this->mousePos = event->pos();
    update();
}

void CCWorkpiece3d::mousePressEvent(QMouseEvent *event) {
    this->mousePos = event->pos();
    if (event->buttons() & Qt::LeftButton && !this->panel->underMouse()) {
        this->trackBall.push(pixelPosToViewPos(event->pos()), this->sceneTrackBall.rotation().conjugated());
    }
    update();
}

void CCWorkpiece3d::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !this->panel->underMouse()) {
        this->trackBall.release(pixelPosToViewPos(event->pos()), this->sceneTrackBall.rotation().conjugated());
    }
    update();
}

void CCWorkpiece3d::wheelEvent(QWheelEvent * event) {
    if(!this->panel->underMouse()){
        this->distExp += event->delta();
        if (this->distExp < -2800)
            this->distExp = -2800;
        if (this->distExp > 800)
            this->distExp = 800;
        update();
    }
}

void CCWorkpiece3d::changeTextureSize(unsigned int width, unsigned int height, unsigned int depth) {

    this->volumeWidth = width;
    this->volumeHeight = height;
    this->volumeDepth = depth;
    if(!this->initialized){
        this->changeTextureSizeFlag = true;
        return;
    }

    makeCurrent();
    this->raycastingVolume->changeTextureSize(width, height, depth);
    doneCurrent();
    this->panel->updateDisplayParameters();

    emit registerBufferCudaGL(this->raycastingVolume->getVolumeTexture(), m_currentScreenNum, true);
}

void CCWorkpiece3d::createOpenGLContextForProcessing(QOpenGLContext *processingContext, QOffscreenSurface *processingSurface, QThread *processingThread) {
    QOpenGLContext* renderContext = this->context();
    QMutexLocker locker(&OCTGlobal::glContextMtx);
    (processingContext)->setFormat(renderContext->format());
    (processingContext)->setShareContext(renderContext);
    (processingContext)->create();
    (processingContext)->moveToThread(processingThread);
    (processingSurface)->setFormat(renderContext->format());
    (processingSurface)->create();//Due to the fact that QOffscreenSurface is backed by a QWindow on some platforms, cross-platform applications must ensure that create() is only called on the main (GUI) thread
    (processingSurface)->moveToThread(processingThread);
    this->changeTextureSize(this->volumeWidth, this->volumeHeight, this->volumeDepth);
}

void CCWorkpiece3d::registerOpenGLBufferWithCuda() {
    changeTextureSize(this->volumeWidth, this->volumeHeight, this->volumeDepth);
}

void CCWorkpiece3d::updateDisplayParams(GLWindow3DParams params) {
    this->displayParams = params;
    this->setMode(params.displayMode);
    this->setStepLength(params.rayMarchStepLength);
    this->setThreshold(params.threshold);
    this->setStretch(params.stretchX, params.stretchY, params.stretchZ);
    this->gamma = params.gamma;
    this->depthWeight = params.depthWeight;
    this->smoothFactor = params.smoothFactor;
    this->alphaExponent = params.alphaExponent;
    this->shadingEnabled = params.shading;
    this->lutEnabled = params.lutEnabled;
}

void CCWorkpiece3d::openLUTFromImage(QImage lut){
    makeCurrent();
    this->raycastingVolume->setLUT(lut);
    doneCurrent();
    this->lutEnabled = true;
}

void CCWorkpiece3d::delayedUpdate() {
    this->update();
    this->delayedUpdatingRunning = false;
}

void CCWorkpiece3d::saveSettings() {
    Settings::getInstance()->storeSettings(this->getName(), this->getSettings());
}

void CCWorkpiece3d::addShader(const QString& name, const QString& vertex, const QString& fragment) {
    this->shaders[name] = new QOpenGLShaderProgram(this);
    this->shaders[name]->addShaderFromSourceFile(QOpenGLShader::Vertex, vertex);
    this->shaders[name]->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment);
    this->shaders[name]->link();
}

void CCWorkpiece3d::restoreLUTSettingsFromPreviousSession() {
    QString fileName = this->displayParams.lutFileName;
    QImage lut(":/luts/hotter_lut.png"); //deafult lut
    if(fileName != ""){
        QImage lutFromFile(fileName);
        if(lutFromFile.isNull()){
            qDebug() << tr("Could not load LUT! File: ") + fileName;
            this->panel->eraseLUTFileName();
        }else{
            lut = lutFromFile;
        }
        //emit info(tr("LUT for volume rendering loaded! File used: ") + fileName);
    }
    makeCurrent();
    this->raycastingVolume->setLUT(lut);
    doneCurrent();
}
