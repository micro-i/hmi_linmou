﻿/**
**  This file is part of OCTproZ.
**  OCTproZ is an open source software for processig of optical
**  coherence tomography (OCT) raw data.
**  Copyright (C) 2019-2022 Miroslav Zabic
**
**  OCTproZ is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program. If not, see http://www.gnu.org/licenses/.
**
****
** Author:	Miroslav Zabic
** Contact:	zabic
**			at
**			spectralcode.de
****
**/

#include "glwindow2d.h"

#include <QTimer>
#include <QDebug>
#include <QMutex>
#include <QApplication>
#include <QString>

#include "oct_algorithm/oct_params/settings.h"
#include "infrastructure/OCTGlobal.h"

const bool test = true;


GLWindow2D::GLWindow2D(QWidget *parent) : QOpenGLWidget(parent) {
    this->width = DEFAULT_WIDTH;
    this->height = DEFAULT_HEIGHT;
    this->depth = 0;
    this->scaleFactor = 1.0;
    this->screenWidthScaled = 1.0;
    this->screenHeightScaled = 1.0;
    this->xTranslation = 0.0;
    this->yTranslation = 0.0;
    this->setMinimumWidth(448);
    this->setMinimumHeight(448);
    this->initialized = false;
    this->changeBufferSizeFlag = false;
    this->keepAspectRatio = true;
    this->delayedUpdatingRunning = false;
    this->frameNr = 0;
    this->rotationAngle = 0.0;
    this->stretchX = 1.0;
    this->stretchY = 1.0;
    this->markerEnabled = true;
    this->coordinateEnabled = true;
    this->setMarkerOrigin(LEFT);
    this->setMarkerPosition(0);
    this->setFocusPolicy(Qt::StrongFocus);
    this->panel = new ControlPanel2D(this);
    this->layout = new QVBoxLayout(this);
    this->layout->addStretch();
    this->layout->addWidget(this->panel);
    this->panel->setVisible(false);

    this->horizontalScaleBar = new ScaleBar();
    this->horizontalScaleBar->setOrientation(ScaleBar::Horizontal);

    this->verticalScaleBar = new ScaleBar();
    this->verticalScaleBar->setOrientation(ScaleBar::Vertical);;

    this->initContextMenu();


    connect(this->panel->spinBoxFrame, QOverload<int>::of(&QSpinBox::valueChanged), this, &GLWindow2D::currentFrameNr);

    if(!test){
        connect(this->panel->doubleSpinBoxStretchX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GLWindow2D::setStretchX);
        connect(this->panel->doubleSpinBoxStretchY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GLWindow2D::setStretchY);
        connect(this->panel->doubleSpinBoxRotationAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GLWindow2D::setRotationAngle);
        connect(this->panel->checkBoxHorizontalScaleBar, &QCheckBox::stateChanged, this->horizontalScaleBar, &ScaleBar::enable);
        connect(this->panel->checkBoxVerticalScaleBar, &QCheckBox::stateChanged, this->verticalScaleBar, &ScaleBar::enable);
        connect(this->panel->spinBoxHorizontalScaleBar, QOverload<int>::of(&QSpinBox::valueChanged), this->horizontalScaleBar, &ScaleBar::setLength);
        connect(this->panel->spinBoxVerticalScaleBar, QOverload<int>::of(&QSpinBox::valueChanged), this->verticalScaleBar, &ScaleBar::setLength);
        connect(this->panel->lineEditHorizontalScaleBarText, &QLineEdit::textChanged, this->horizontalScaleBar, &ScaleBar::setText);
        connect(this->panel->lineEditVerticalScaleBarText, &QLineEdit::textChanged, this->verticalScaleBar, &ScaleBar::setText);
    }
    //connect(this->panel, &ControlPanel2D::settingsChanged, this, &GLWindow2D::saveSettings);
}


GLWindow2D::~GLWindow2D()
{
    this->saveSettings();
    delete this->horizontalScaleBar;
    delete this->verticalScaleBar;
    //todo: check if cleanup (probably for processingContext and processingSurface) is necessary and implement it
}

void GLWindow2D::setMarkerOrigin(FRAME_EDGE origin) {
    this->markerOrigin = origin;
}

// 传参给控制面板
void GLWindow2D::setSettings(QVariantMap settings) {
    GLWindow2DParams params;
    params.extendedViewEnabled = settings.value(EXTENDED_PANEL).toBool();
    params.displayedFrames = settings.value(DISPLAYED_FRAMES).toInt();
    params.currentFrame = settings.value(CURRENT_FRAME).toInt();
    params.rotationAngle = settings.value(ROTATION_ANGLE).toDouble();
    params.displayFunction = settings.value(DISPLAY_MODE).toInt();
    params.stretchX = settings.value(STRETCH_X).toDouble();
    params.stretchY = settings.value(STRETCH_Y).toDouble();
    params.horizontalScaleBarEnabled = settings.value(HORIZONTAL_SCALE_BAR_ENABLED).toBool();
    params.verticalScaleBarEnabled = settings.value(VERTICAL_SCALE_BAR_ENABLED).toBool();
    params.horizontalScaleBarText = settings.value(HORIZONTAL_SCALE_BAR_TEXT).toString();
    params.verticalScaleBarText = settings.value(VERTICAL_SCALE_BAR_TEXT).toString();
    params.horizontalScaleBarLength = settings.value(HORIZONTAL_SCALE_BAR_LENGTH).toInt();
    params.verticalScaleBarLength = settings.value(VERTICAL_SCALE_BAR_LENGTH).toInt();
    this->panel->setParams(params);
}

// 从控制面板获取参数
QVariantMap GLWindow2D::getSettings() {
    QVariantMap settings;
    GLWindow2DParams params = this->panel->getParams();
    settings.insert(EXTENDED_PANEL, params.extendedViewEnabled);
    settings.insert(DISPLAYED_FRAMES, params.displayedFrames);
    settings.insert(CURRENT_FRAME, params.currentFrame);
    settings.insert(ROTATION_ANGLE, params.rotationAngle);
    settings.insert(DISPLAY_MODE, params.displayFunction);
    settings.insert(STRETCH_X, params.stretchX);
    settings.insert(STRETCH_Y, params.stretchY);
    settings.insert(HORIZONTAL_SCALE_BAR_ENABLED, params.horizontalScaleBarEnabled);
    settings.insert(VERTICAL_SCALE_BAR_ENABLED, params.verticalScaleBarEnabled);
    settings.insert(HORIZONTAL_SCALE_BAR_TEXT, params.horizontalScaleBarText);
    settings.insert(VERTICAL_SCALE_BAR_TEXT, params.verticalScaleBarText);
    settings.insert(HORIZONTAL_SCALE_BAR_LENGTH, params.horizontalScaleBarLength);
    settings.insert(VERTICAL_SCALE_BAR_LENGTH, params.verticalScaleBarLength);
    return settings;
}

void GLWindow2D::setScanRange(float screenx, float screeny, bool bscan)
{
    m_xRange = screenx;
    m_yRange = screeny;
    m_isBscan = bscan;
}

// 右键菜单
void GLWindow2D::initContextMenu() {
    this->contextMenu = new QMenu(this);

    this->keepAspectRatioAction = new QAction(tr("Keep &Aspect Ratio"), this);
    this->keepAspectRatioAction->setCheckable(true);
    this->keepAspectRatioAction->setChecked(this->keepAspectRatio);
    connect(this->keepAspectRatioAction, &QAction::toggled, this, &GLWindow2D::setKeepAspectRatio);
    this->contextMenu->addAction(this->keepAspectRatioAction);

    this->markerAction = new QAction(tr("Display orthogonal &marker"), this);
    this->markerAction->setCheckable(true);
    this->markerAction->setChecked(this->markerEnabled);
    connect(this->markerAction, &QAction::toggled, this, &GLWindow2D::enableMarker);
    contextMenu->addAction(this->markerAction);

    this->coordAction = new QAction(tr("Display coordinate"));
    this->coordAction->setCheckable(true);
    this->coordAction->setChecked(this->markerEnabled);
    connect(this->coordAction, &QAction::toggled, this, &GLWindow2D::enableCoordinate);
    contextMenu->addAction(this->coordAction);

    this->screenshotAction = new QAction(tr("&Screenshot..."), this);
    connect(this->screenshotAction, &QAction::triggered, this, &GLWindow2D::openScreenshotDialog);
    this->contextMenu->addAction(this->screenshotAction);
}

// 显示ScaleBar
void GLWindow2D::displayScalebars() {
    if(this->horizontalScaleBar->isEnabled()){
        float horizontalScaleFactor = this->scaleFactor*this->stretchX*static_cast<float>(this->size().width())*(this->screenWidthScaled)/static_cast<float>(this->height);
        this->horizontalScaleBar->draw(this, horizontalScaleFactor);
    }
    if(this->verticalScaleBar->isEnabled()){
        float verticalScaleFactor = this->scaleFactor*this->stretchY*static_cast<float>(this->size().height())*(this->screenHeightScaled)/static_cast<float>(this->width);
        this->verticalScaleBar->draw(this, verticalScaleFactor);
    }
}

void GLWindow2D::displayMarker() {
    if(this->markerEnabled){
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glColor3f(0.8f, 0.0f, 0.0f);
        glVertex3f(this->markerCoordinates.x1, this->markerCoordinates.y1, 0.0);
        glVertex3f(this->markerCoordinates.x2, this->markerCoordinates.y2, 0.0);
        glEnd();
    }
    if(this->coordinateEnabled){
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glColor3f(0.8f, 0.8f, 0.8f);
        // square
        glVertex3f(-this->screenWidthScaled, -this->screenHeightScaled, 0.0);
        glVertex3f(this->screenWidthScaled, -this->screenHeightScaled, 0.0);

        glVertex3f(this->screenWidthScaled, -this->screenHeightScaled, 0.0);
        glVertex3f(this->screenWidthScaled, this->screenHeightScaled, 0.0);

        glVertex3f(this->screenWidthScaled, this->screenHeightScaled, 0.0);
        glVertex3f(-this->screenWidthScaled, this->screenHeightScaled, 0.0);

        glVertex3f(-this->screenWidthScaled, this->screenHeightScaled, 0.0);
        glVertex3f(-this->screenWidthScaled, -this->screenHeightScaled, 0.0);

        if(m_xRange <=0 || m_yRange <= 0){
            glEnd();
            return;
        }

        float widthPer = this->screenWidthScaled / width * 200;
        float heightPer = this->screenHeightScaled / height * 200;

        for(int i=1; i<10; i++){
            glVertex3f(-this->screenWidthScaled, this->screenHeightScaled - i * heightPer, 0.0);
            glVertex3f(-this->screenWidthScaled + widthPer / 8.0, this->screenHeightScaled - i * heightPer, 0.0);

            glVertex3f(this->screenWidthScaled, this->screenHeightScaled -  i* heightPer , 0.0);
            glVertex3f(this->screenWidthScaled - widthPer / 8.0, this->screenHeightScaled - i * heightPer, 0.0);

            glVertex3f(-this->screenWidthScaled + i * widthPer, -this->screenHeightScaled, 0.0);
            glVertex3f(-this->screenWidthScaled + i * widthPer, -this->screenHeightScaled + heightPer / 8.0, 0.0);

            glVertex3f(-this->screenWidthScaled + i * widthPer, this->screenHeightScaled, 0.0);
            glVertex3f(-this->screenWidthScaled + i * widthPer, this->screenHeightScaled - heightPer / 8.0, 0.0);
        }
        glEnd();
        QFont f;
        f.setPixelSize(10);
        renderText(-this->screenWidthScaled + 5 * widthPer, this->screenHeightScaled + heightPer, 0.0, QString("unit (mm) X-%1").arg(m_isBscan?"Z":"Y"), f);
        float perX = m_xRange / 10;
        float perY = m_yRange / 10;
        for(int i=0; i<11; i++){
            renderText(-this->screenWidthScaled - widthPer, this->screenHeightScaled - i*heightPer, 0.0, QString::number(i * perY), f);
            renderText(-this->screenWidthScaled + i * widthPer, this->screenHeightScaled + heightPer / 3, 0.0, QString::number(i * perX), f);
        }
    }
}

void GLWindow2D::displayOrientationLine(int x, int y, int length) {
    int distanceFromVerticalEdgeInPx = x;
    int distanceFromHorizontalEdgeInPx = y;
    float distanceFromEdgeX = (this->screenWidthScaled*distanceFromVerticalEdgeInPx)/static_cast<float>(this->height);
    float distanceFromEdgeY = (this->screenHeightScaled*distanceFromHorizontalEdgeInPx)/static_cast<float>(this->width);
    float normalizedLength = (2.0f*this->screenWidthScaled*static_cast<float>(length))/static_cast<float>(this->height); //this ist just vor horizontal lines
    glLineWidth(30.0);
    glBegin(GL_LINES);
    glColor4f(0.8f, 0.8f, 0.8f, 0.8f); //hint: to use alpha channel blending needs to be enabeled in initializeGL
    glVertex3f(-this->screenWidthScaled+distanceFromEdgeX, (this->screenHeightScaled-distanceFromEdgeY), 0.0);
    glVertex3f(-this->screenWidthScaled+distanceFromEdgeX+normalizedLength,(this->screenHeightScaled-distanceFromEdgeY), 0.0);
    glEnd();
}

void GLWindow2D::delayedUpdate() {
    this->update();
    this->delayedUpdatingRunning = false;
}

void GLWindow2D::changeTextureSize(unsigned int width, unsigned int height, unsigned int depth) {
    this->width = width <= 1 ? 128 : width; //width and height shall not be zero. 128 is an arbitrary value greater 1 and a power of 2
    this->height = height <= 1 ? 128 : height;
    this->depth = depth;
    this->panel->setMaxFrame(depth-1);
    this->panel->setMaxAverage(depth-1);
    qDebug() << this->getName()  << "width: " << this->width << " height: " << this->height << " depth: " << this->depth;
    if(!this->initialized){
        this->changeBufferSizeFlag = true;
        return;
    }
    this->resizeGL((float)this->size().width(),(float)this->size().height());

    makeCurrent();
    glDeleteBuffers(1, &(this->buf));
    glGenBuffers(1, &(this->buf));
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, this->buf);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, (this->width * this->height * sizeof(float)), 0, GL_DYNAMIC_COPY);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    doneCurrent();
    emit registerBufferCudaGL(this->buf);

    this->setMarkerPosition(this->markerPosition);
}

void GLWindow2D::createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread) {
    QOpenGLContext* renderContext = this->context();
    //    renderContext->create();
    QMutexLocker lck(&OCTGlobal::glContextMtx);
    (processingContext)->setFormat(renderContext->format());
    (processingContext)->setShareContext(renderContext);
    (processingContext)->create();
    (processingContext)->moveToThread(processingThread);
    (processingSurface)->setFormat(renderContext->format());
    (processingSurface)->create(); //Due to the fact that QOffscreenSurface is backed by a QWindow on some platforms, cross-platform applications must ensure that create() is only called on the main (GUI) thread
    (processingSurface)->moveToThread(processingThread);

    this->changeTextureSize(this->width, this->height, this->depth);

    //QOpenGLContext::areSharing(processingContext, renderContext) ? emit info("processingContext, renderContext: yes") : emit info("processingContext, renderContext: no");
}

void GLWindow2D::registerOpenGLBufferWithCuda() {
    emit registerBufferCudaGL(this->buf);
}

void GLWindow2D::setKeepAspectRatio(bool enable) {
    this->keepAspectRatio = enable;
    this->resizeGL(this->size().width(),this->size().height());
}

void GLWindow2D::setRotationAngle(float angle) {
    this->rotationAngle = angle;
    this->resizeGL(this->size().width(), this->size().height());
}

void GLWindow2D::setStretchX(float stretchFactor) {
    this->stretchX = stretchFactor;
    this->resizeGL(this->size().width(), this->size().height());
}

void GLWindow2D::setStretchY(float stretchFactor) {
    this->stretchY = stretchFactor;
    this->resizeGL(this->size().width(),this->size().height());
}

void GLWindow2D::openScreenshotDialog() {
    QImage screenshot = this->grabFramebuffer();
    emit dialogAboutToOpen();
    QCoreApplication::processEvents();
    QString filters("(*.png);;(*.jpg);;(*.bmp)");
    QString defaultFilter("(*.png)");
    QString path = OCTGlobal::lastSelectedFolder;
    QDir dir(path);
    if(!dir.exists()){
        path = QDir::currentPath();
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save screenshot to..."), path, filters, &defaultFilter);
    emit dialogClosed();
    if(fileName == ""){
        return;
    }
    QFileInfo finfo(fileName);
    OCTGlobal::lastSelectedFolder = finfo.dir().absolutePath();
    qDebug() << OCTGlobal::lastSelectedFolder;
    bool saved = false;
    saved = screenshot.save(fileName);
    if(saved){
        emit info(tr("Screenshot saved to ") + fileName);
    }else{
        emit error(tr("Could not save screenshot to disk."));
    }
}

void GLWindow2D::enableMarker(bool enable) {
    this->markerEnabled = enable;
}

void GLWindow2D::enableCoordinate(bool enable){
    this->coordinateEnabled = enable;
}

void GLWindow2D::setMarkerPosition(unsigned int position) {
    this->markerPosition = position;
    float linePos = 0;
    float markerTexturePosition = 0;
    switch(this->markerOrigin){
    case TOP:
        linePos = (2.0f*this->markerPosition)/static_cast<float>(this->width)-1.0f;
        markerTexturePosition = this->screenHeightScaled*linePos;
        this->markerCoordinates.x1 = this->screenWidthScaled;
        this->markerCoordinates.x2 = -this->screenWidthScaled;
        this->markerCoordinates.y1 = -markerTexturePosition;
        this->markerCoordinates.y2 = -markerTexturePosition;
        break;
    case BOTTOM:
        linePos = (2.0f*this->markerPosition)/static_cast<float>(this->width)-1.0f;
        markerTexturePosition = this->screenHeightScaled*linePos;
        this->markerCoordinates.x1 = this->screenWidthScaled;
        this->markerCoordinates.x2 = -this->screenWidthScaled;
        this->markerCoordinates.y1 = markerTexturePosition;
        this->markerCoordinates.y2 = markerTexturePosition;
        break;
    case LEFT:
        linePos = (2.0f*this->markerPosition)/static_cast<float>(this->height)-1.0f;
        markerTexturePosition = this->screenWidthScaled*linePos;
        this->markerCoordinates.x1 = markerTexturePosition;
        this->markerCoordinates.x2 = markerTexturePosition;
        this->markerCoordinates.y1 = this->screenHeightScaled;
        this->markerCoordinates.y2 = -this->screenHeightScaled;
        break;
    case RIGHT:
        linePos = (2.0f*this->markerPosition)/static_cast<float>(this->height)-1.0f;
        markerTexturePosition = this->screenWidthScaled*linePos;
        this->markerCoordinates.x1 = -markerTexturePosition;
        this->markerCoordinates.x2 = -markerTexturePosition;
        this->markerCoordinates.y1 = this->screenHeightScaled;
        this->markerCoordinates.y2 = -this->screenHeightScaled;
        break;
    }
}

void GLWindow2D::saveSettings() {
    Settings::getInstance()->storeSettings(this->getName(), this->getSettings());
}

void GLWindow2D::saveScreenshot(QString savePath, QString fileName) {
    QImage screenshot = this->grabFramebuffer();
    QString filePath = savePath + "/" + fileName;
    screenshot.save(filePath);
    emit info(tr("Screenshot saved to ") + filePath);
}

void GLWindow2D::initializeGL() {
    initializeOpenGLFunctions();

    //check if width and height are greater 1. if not, set them to the (arbitrary) value 128
    if(this->width <= 1){
        this->width = 128;
    }
    if(this->height <= 1){
        this->height = 128;
    }

    glGenBuffers(1, &buf);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, buf);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, (this->width * this->height * sizeof(float)), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glGenTextures(1, &(this->texture));
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //enable blending //may be used in future for transparent user defined orientation lines
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);

    if(this->initialized){
        emit registerBufferCudaGL(this->buf); //registerBufferCudaGL is necessary here because as soon as the openglwidget/dock is removed from the main window initializeGL() is called again.
    }

    this->initialized = true;

    if(this->changeBufferSizeFlag){
        this->changeTextureSize(this->width, this->height, this->depth);
        this->changeBufferSizeFlag = false;
    }
}

void GLWindow2D::paintGL() {
    //reset matrix state (resets previous translation, rotation and scale operations)
    glLoadIdentity();

    //translation
    glTranslatef(this->xTranslation, this->yTranslation, 0);

    //rotation
    if(this->keepAspectRatio){
        float screenWidth = static_cast<float>(this->size().width());
        float screenHeight = static_cast<float>(this->size().height());
        glScalef(1.0f,screenWidth/screenHeight,1.0f);
        glRotatef(this->rotationAngle, 0.0, 0.0, 1.0);
        glScalef(1.0f,screenHeight/screenWidth,1.0f);
    }else{
        glRotatef(this->rotationAngle, 0.0, 0.0, 1.0);
    }

    //zoom and stretch
    glScalef(this->scaleFactor*this->stretchX, this->scaleFactor*this->stretchY, 0.f);

    //display oct data
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, this->buf);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 1), glVertex2f(-this->screenWidthScaled, -this->screenHeightScaled); //place upper left texture coordinate to bottom left screen position
    glTexCoord2f(1, 1), glVertex2f(-this->screenWidthScaled, this->screenHeightScaled); //upper right texture coordinate to upper left screen position
    glTexCoord2f(1, 0), glVertex2f(this->screenWidthScaled, this->screenHeightScaled); //bottom right texture coordinate to upper right screen position
    glTexCoord2f(0, 0), glVertex2f(this->screenWidthScaled, -this->screenHeightScaled); //bottom left texture coordinate to bottom right screen position

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    //display optional overlays
    this->displayMarker();
    this->displayScalebars();
    //this->displayOrientationLine(0, 250, 256); //todo: implement orientation line feature (Line that can be placed by the user at a desired position over the b-scan / en face view)

    if(!this->delayedUpdatingRunning){
        this->delayedUpdatingRunning = true;
        QTimer::singleShot(DELAY_TIME_IN_ms, this, QOverload<>::of(&GLWindow2D::delayedUpdate));
    }
}

void GLWindow2D::resizeGL(int w, int h) {
    this->screenWidthScaled = 1.0f;
    this->screenHeightScaled = 1.0f;
    if(this->keepAspectRatio){
        float textureWidth = static_cast<float>(this->height);
        float textureHeight = static_cast<float>(this->width);
        float screenWidth = static_cast<float>(w);// this is the same as static_cast<float>(this->size().width());
        float screenHeight = static_cast<float>(h);//this is the same as static_cast<float>(this->size().height());
        float ratioTexture = textureWidth/textureHeight;
        float ratioScreen = screenWidth/screenHeight;
        if(ratioTexture > ratioScreen){
            this->screenHeightScaled = (screenWidth/ratioTexture)/screenHeight;
        }
        else{
            this->screenWidthScaled = (screenHeight*ratioTexture)/screenWidth;
        }
    }
    //recalculate marker coordinates
    this->setMarkerPosition(this->markerPosition);
}

void GLWindow2D::mousePressEvent(QMouseEvent* event) {
    this->mousePos = event->pos();
}

void GLWindow2D::mouseMoveEvent(QMouseEvent* event) {
    if(event->buttons() & Qt::LeftButton && !this->panel->underMouse()){
        QPoint delta = (event->pos() - this->mousePos);
        int windowWidth = this->size().width();
        int windowHeight = this->size().height();
        this->xTranslation += 2.0*(float)delta.x()/((float)windowWidth);
        this->yTranslation += -2.0*(float)delta.y()/(float)windowHeight;
    }
    this->mousePos = event->pos();
}

void GLWindow2D::wheelEvent(QWheelEvent *event) {
    if(!this->panel->underMouse()){
        QPoint numPixels = event->pixelDelta();
        QPoint numDegrees = event->angleDelta()/8;

        if (!numPixels.isNull()) {
            this->scaleFactor += numPixels.y()/30.0;
        } else if (!numDegrees.isNull()) {
            QPoint numSteps = numDegrees/15;
            this->scaleFactor += (float)numSteps.y()/30.0;
            if(this->scaleFactor<0.05){
                this->scaleFactor = 0.05;
            }
        }
        event->accept();
    }
}

void GLWindow2D::mouseDoubleClickEvent(QMouseEvent *event) {
    if(!this->panel->underMouse()){
        this->scaleFactor = 1.0;
        this->xTranslation = 0.0;
        this->yTranslation = 0.0;
    }
}

void GLWindow2D::enterEvent(QEvent *event) {
    this->panel->setVisible(true);
}

void GLWindow2D::leaveEvent(QEvent *event) {
    this->panel->setVisible(false);
}
void GLWindow2D::contextMenuEvent(QContextMenuEvent *event) {
    this->contextMenu->exec(event->globalPos());
}

void GLWindow2D::renderText(double x, double y, double z, const QString &text, const QFont& font)
{
    int width = this->size().width();
    int height = this->size().height();

    GLdouble model[4][4], proj[4][4];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);
    GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

    project(x, y, z,
            &model[0][0], &proj[0][0], &view[0],
            &textPosX, &textPosY, &textPosZ);

    textPosY = height - textPosY; // y is inverted

    QPainter painter(this);
    painter.setFont(font);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(textPosX, textPosY, text); // z = pointT4.z + distOverOp / 4
    painter.end();
}

inline GLint GLWindow2D::project(GLdouble objx, GLdouble objy, GLdouble objz,
                                 const GLdouble model[16], const GLdouble proj[16],
const GLint viewport[4],
GLdouble * winx, GLdouble * winy, GLdouble * winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;
    transformPoint(out, model, in);
    transformPoint(in, proj, out);

    if (in[3] == 0.0)
        return GL_FALSE;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

    *winz = (1 + in[2]) / 2;
    return GL_TRUE;
}

inline void GLWindow2D::transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
    out[0] =
            M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
    out[1] =
            M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
    out[2] =
            M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
    out[3] =
            M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}





//_______________________

ScaleBar::ScaleBar() : QObject(), QPainter() {
    this->enable(false);
    this->setOrientation(Horizontal);
    this->setPos(QPoint(20, 20));
    this->setLength(256);
    this->setText("1 mm");
    this->textDistanceToScaleBarInPx = 2;
}

ScaleBar::~ScaleBar()
{
}

void ScaleBar::setText(QString text) {
    if(this->text != text){
        this->text = text;
        this->textChanged = true;
    }
}

void ScaleBar::draw(QPaintDevice* paintDevice, float scaleFactor) {
    this->begin(paintDevice);
    if(this->textChanged){
        this->updateTextSizeInfo();
        this->textChanged = false;
    }
    if(this->orientation == Horizontal){
        //calculate coordinates for horizontal scale bar and text
        float xEnd = this->pos.x()+scaleFactor*this->lenghInPx;
        float textPosX = this->pos.x()+(xEnd - this->pos.x())/2.0-this->textWidth/2.0;
        float textPosY = (this->pos.y()+this->textHeight)+this->textDistanceToScaleBarInPx-4; //textHeight seems to be 4 px bigger than the acutal text height
        //draw horizontal scale bar and text
        this->drawLine(this->pos.x(), this->pos.y(), xEnd, this->pos.y());
        this->drawText(textPosX, textPosY, this->text);
    }else{
        //calculate coordinates for vertical scale bar and text
        float yEnd = this->pos.y()+scaleFactor*this->lenghInPx;
        float textPosX = this->pos.x()+this->textDistanceToScaleBarInPx;
        float textPosY = this->pos.y()+(yEnd - this->pos.y())/2.0+this->textHeight/2.0;
        //draw vertical scale bar and text
        this->drawLine(this->pos.x(), this->pos.y(), this->pos.x(), this->pos.y()+scaleFactor*this->lenghInPx);
        this->drawText(textPosX, textPosY, this->text);
    }
    this->end();
}

void ScaleBar::updateTextSizeInfo() {
    QFontMetrics fm = this->fontMetrics();
    this->textWidth = fm.boundingRect(text).width();
    this->textHeight = fm.boundingRect(text).height();
}

//_______________________








//_______________________
ControlPanel2D::ControlPanel2D(QWidget *parent) : QWidget(parent) {
    this->panel = new QWidget(parent);
    this->panel->setWindowFlags(Qt::FramelessWindowHint);
    this->setProperty("style", "ControlPanel");
    this->panel->setProperty("style", "ControlPanel");
    this->widgetLayout = new QHBoxLayout(this);
    this->widgetLayout->addWidget(this->panel);
    this->spinBoxAverage = new QSpinBox(this->panel);
    this->spinBoxFrame = new QSpinBox(this->panel);
    this->spinBoxFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->labelFrame = new QLabel(tr("Display frame:"), this->panel);

    this->slider = new QSlider(Qt::Horizontal, this->panel);
    this->slider->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    this->stringBoxFunctions = new StringSpinBox(this->panel);
    this->labelDisplayFunction = new QLabel(tr("Display mode:"), this->panel);
    this->labelDisplayFunctionFrames = new QLabel(tr("Frames:"), this->panel);

    this->layout = new QGridLayout(this->panel);
    this->layout->setContentsMargins(0,0,0,0);
    this->layout->setVerticalSpacing(1);

    if(test){

        this->layout->addWidget(this->labelFrame, 0, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->slider, 0, 1, 1, 2);
        this->layout->setColumnStretch(2, 100); //set high stretch factor such that slider gets max available space

        this->layout->addWidget(this->spinBoxFrame, 0, 3, 1, 1, Qt::AlignLeft);

        this->layout->addWidget(this->labelDisplayFunction, 1, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->stringBoxFunctions, 1, 1, 1, 1, Qt::AlignLeft);
        this->layout->addWidget(this->labelDisplayFunctionFrames, 1, 2, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->spinBoxAverage, 1, 3, 1, 1, Qt::AlignLeft);
    }else{
        this->labelRotationAngle = new QLabel(tr("Rotation:"), this->panel);
        this->doubleSpinBoxRotationAngle = new QDoubleSpinBox(this->panel);
        this->doubleSpinBoxRotationAngle->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        this->toolButtonMore = new QToolButton(this->panel);
        this->toolButtonMore->setArrowType(Qt::DownArrow);
        this->toolButtonMore->setAutoRaise(true);

        this->doubleSpinBoxStretchX = new QDoubleSpinBox(this->panel);
        this->labelStretchX = new QLabel(tr("Stretch X:"), this->panel);
        this->doubleSpinBoxStretchY = new QDoubleSpinBox(this->panel);
        this->labelStretchY = new QLabel(tr("Stretch Y:"), this->panel);

        this->checkBoxHorizontalScaleBar = new QCheckBox(tr("X scale"), this->panel);
        this->labelHorizontalScaleBar = new QLabel(tr("Length in px:"), this->panel);
        this->labelHorizontalScaleBarText = new QLabel(tr("Text:"), this->panel);
        this->lineEditHorizontalScaleBarText = new QLineEdit(this->panel);
        this->checkBoxVerticalScaleBar = new QCheckBox(tr("Y scale"), this->panel);
        this->labelVerticalScaleBar = new QLabel(tr("Length in px:"), this->panel);
        this->labelVerticalScaleBarText = new QLabel(tr("Text:"), this->panel);
        this->lineEditVerticalScaleBarText = new QLineEdit(this->panel);
        this->spinBoxHorizontalScaleBar = new QSpinBox(this->panel);
        this->spinBoxVerticalScaleBar = new QSpinBox(this->panel);


        this->layout->addWidget(this->labelFrame, 0, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->slider, 0, 1, 1, 5);
        this->layout->setColumnStretch(4, 100); //set high stretch factor such that slider gets max available space
        this->layout->addWidget(this->spinBoxFrame, 0, 6, 1, 1, Qt::AlignLeft);
        this->layout->addWidget(this->labelDisplayFunction, 1, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->stringBoxFunctions, 1, 1, 1, 1, Qt::AlignLeft);
        this->layout->addWidget(this->labelDisplayFunctionFrames, 1, 2, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->spinBoxAverage, 1, 3, 1, 1, Qt::AlignLeft);

        this->layout->addWidget(this->toolButtonMore, 1, 4, 1, 1, Qt::AlignCenter);

        this->layout->addWidget(this->labelStretchX, 3, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->doubleSpinBoxStretchX,3, 1, 1, 1);
        this->layout->addWidget(this->labelStretchY, 4, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->doubleSpinBoxStretchY,4, 1, 1, 1);
        this->layout->addWidget(this->labelRotationAngle, 5, 0, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->doubleSpinBoxRotationAngle, 5, 1, 1, 1);

        this->layout->addWidget(this->checkBoxHorizontalScaleBar, 3, 2, 1, 1, Qt::AlignLeft);
        this->layout->addWidget(this->labelHorizontalScaleBar, 3, 3, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->spinBoxHorizontalScaleBar, 3, 4, 1, 1);
        this->layout->addWidget(this->labelHorizontalScaleBarText, 3, 5, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->lineEditHorizontalScaleBarText, 3, 6, 1, 1);

        this->layout->addWidget(this->checkBoxVerticalScaleBar, 4, 2, 1, 1, Qt::AlignLeft);
        this->layout->addWidget(this->labelVerticalScaleBar, 4, 3, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->spinBoxVerticalScaleBar, 4, 4, 1, 1);
        this->layout->addWidget(this->labelVerticalScaleBarText, 4, 5, 1, 1, Qt::AlignRight);
        this->layout->addWidget(this->lineEditVerticalScaleBarText, 4, 6, 1, 1);

        this->doubleSpinBoxStretchX->setMaximum(9999);
        this->doubleSpinBoxStretchX->setMinimum(0.1);
        this->doubleSpinBoxStretchX->setSingleStep(0.1);
        this->doubleSpinBoxStretchX->setValue(1.0);

        this->doubleSpinBoxStretchY->setMaximum(9999);
        this->doubleSpinBoxStretchY->setMinimum(0.1);
        this->doubleSpinBoxStretchY->setSingleStep(0.1);
        this->doubleSpinBoxStretchY->setValue(1.0);

        this->doubleSpinBoxRotationAngle->setMinimum(-360.0);
        this->doubleSpinBoxRotationAngle->setMaximum(360.0);
        this->doubleSpinBoxRotationAngle->setDecimals(1);
        this->doubleSpinBoxRotationAngle->setSingleStep(1.0);

        this->spinBoxHorizontalScaleBar->setMinimum(1);
        this->spinBoxHorizontalScaleBar->setMaximum(9999);
        this->spinBoxHorizontalScaleBar->setSingleStep(1);

        this->spinBoxVerticalScaleBar->setMinimum(1);
        this->spinBoxVerticalScaleBar->setMaximum(9999);
        this->spinBoxVerticalScaleBar->setSingleStep(1);

        connect(this->toolButtonMore, &QToolButton::clicked, this, &ControlPanel2D::toggleExtendedView);
    }


    this->spinBoxAverage->setMinimum(1);
    this->spinBoxAverage->setSingleStep(1);
    this->spinBoxFrame->setMinimum(0);
    this->spinBoxFrame->setSingleStep(1);
    this->slider->setMinimum(0);
    QStringList displayFunctionOptions = { "Averaging", "MIP"}; //todo: think of better way to add available display techniques
    this->stringBoxFunctions->setStrings(displayFunctionOptions);

    this->setMaxFrame(4095);

    connect(this->slider, &QSlider::valueChanged, this->spinBoxFrame, &QSpinBox::setValue);
    connect(this->spinBoxFrame, QOverload<int>::of(&QSpinBox::valueChanged), this->slider, &QSlider::setValue);
    connect(this->slider, &QSlider::valueChanged, this, &ControlPanel2D::updateDisplayFrameSettings);
    connect(this->spinBoxAverage, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel2D::updateDisplayFrameSettings);
    connect(this->stringBoxFunctions, &StringSpinBox::indexChanged, this, &ControlPanel2D::updateDisplayFrameSettings);


    this->enableExtendedView(false);
    this->findGuiElements();
    this->spinBoxes.removeOne(this->spinBoxFrame);//remove frame spinbox to avoid updating settings.ini file every single time the current frame is changed by user
    this->connectGuiToSettingsChangedSignal();
    this->panel->setStyleSheet("QWidget{background-color: #272A33; color: rgba(255, 255, 255, 1);}"
                               "QAbstractSpinBox{background-color: rgba(39, 42, 51, 1); color: rgba(255, 255, 255, 1);}");
}

ControlPanel2D::~ControlPanel2D()
{

}

void ControlPanel2D::setMaxFrame(unsigned int maxFrame) {
    this->spinBoxFrame->setMaximum(maxFrame);
    this->slider->setMaximum(maxFrame);
}

void ControlPanel2D::setMaxAverage(unsigned int maxAverage) {
    this->spinBoxAverage->setMaximum(maxAverage);
}

GLWindow2DParams ControlPanel2D::getParams() {
    this->updateParams();
    return this->params;
}

void ControlPanel2D::findGuiElements() {
    this->lineEdits = this->findChildren<QLineEdit*>();
    this->checkBoxes = this->findChildren<QCheckBox*>();
    this->doubleSpinBoxes = this->findChildren<QDoubleSpinBox*>();
    this->spinBoxes = this->findChildren<QSpinBox*>();
    this->stringSpinBoxes = this->findChildren<StringSpinBox*>();
    this->comboBoxes = this->findChildren<QComboBox*>();
}

void ControlPanel2D::enableExtendedView(bool enable) {
    if(!test){
        this->extendedView = enable;
        enable ? this->toolButtonMore->setArrowType(Qt::UpArrow) : this->toolButtonMore->setArrowType(Qt::DownArrow);
        this->doubleSpinBoxStretchX->setVisible(enable);
        this->doubleSpinBoxStretchY->setVisible(enable);
        this->doubleSpinBoxRotationAngle->setVisible(enable);
        this->labelStretchX->setVisible(enable);
        this->labelStretchY->setVisible(enable);
        this->labelRotationAngle->setVisible(enable);
        this->checkBoxHorizontalScaleBar->setVisible(enable);
        this->labelHorizontalScaleBar->setVisible(enable);
        this->labelHorizontalScaleBarText->setVisible(enable);
        this->lineEditHorizontalScaleBarText->setVisible(enable);
        this->checkBoxVerticalScaleBar->setVisible(enable);
        this->labelVerticalScaleBar->setVisible(enable);
        this->labelVerticalScaleBarText->setVisible(enable);
        this->lineEditVerticalScaleBarText->setVisible(enable);
        this->spinBoxHorizontalScaleBar->setVisible(enable);
        this->spinBoxVerticalScaleBar->setVisible(enable);
    }
}

void ControlPanel2D::updateParams() {
    this->params.displayedFrames = this->spinBoxAverage->value(); //todo: rename spinBoxAverage to spinBoxDisplayedFrames
    this->params.currentFrame = this->spinBoxFrame->value();
    this->params.displayFunction = this->stringBoxFunctions->getIndex();
    if(!test){
        this->params.rotationAngle = this->doubleSpinBoxRotationAngle->value();
        this->params.extendedViewEnabled = this->extendedView; //todo: avoid redundant bools
        this->params.stretchX = this->doubleSpinBoxStretchX->value();
        this->params.stretchY = this->doubleSpinBoxStretchY->value();
        this->params.horizontalScaleBarEnabled = this->checkBoxHorizontalScaleBar->isChecked();
        this->params.verticalScaleBarEnabled = this->checkBoxVerticalScaleBar->isChecked();
        this->params.horizontalScaleBarText = this->lineEditHorizontalScaleBarText->text();
        this->params.verticalScaleBarText = this->lineEditVerticalScaleBarText->text();
        this->params.horizontalScaleBarLength = this->spinBoxHorizontalScaleBar->value();
        this->params.verticalScaleBarLength = this->spinBoxVerticalScaleBar->value();
    }
}

void ControlPanel2D::connectGuiToSettingsChangedSignal() {
    foreach(auto element,this->spinBoxes) {
        connect(element, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->doubleSpinBoxes) {
        connect(element, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->stringSpinBoxes) {
        connect(element, &StringSpinBox::indexChanged, this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->lineEdits) {
        connect(element, &QLineEdit::textChanged, this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->checkBoxes) {
        connect(element, &QCheckBox::clicked, this, &ControlPanel2D::settingsChanged);
    }
}

void ControlPanel2D::disconnectGuiFromSettingsChangedSignal() {
    foreach(auto element,this->spinBoxes) {
        disconnect(element, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->doubleSpinBoxes) {
        disconnect(element, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->stringSpinBoxes) {
        disconnect(element, &StringSpinBox::indexChanged, this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->lineEdits) {
        disconnect(element, &QLineEdit::textChanged, this, &ControlPanel2D::settingsChanged);
    }
    foreach(auto element,this->checkBoxes) {
        disconnect(element, &QCheckBox::clicked, this, &ControlPanel2D::settingsChanged);
    }
}

void ControlPanel2D::setParams(GLWindow2DParams params) {
    this->disconnectGuiFromSettingsChangedSignal();
    this->params = params;
    this->spinBoxAverage->setValue(this->params.displayedFrames); //todo: rename spinBoxAverage to spinBoxDisplayedFrames
    this->spinBoxFrame->setValue(this->params.currentFrame);
    this->stringBoxFunctions->setIndex(this->params.displayFunction);
    if(!test){
        this->enableExtendedView(this->params.extendedViewEnabled);
        this->doubleSpinBoxRotationAngle->setValue(this->params.rotationAngle);
        this->doubleSpinBoxStretchX->setValue(this->params.stretchX);
        this->doubleSpinBoxStretchY->setValue(this->params.stretchY);
        this->checkBoxHorizontalScaleBar->setChecked(this->params.horizontalScaleBarEnabled);
        this->checkBoxVerticalScaleBar->setChecked(this->params.verticalScaleBarEnabled);
        this->lineEditVerticalScaleBarText->setText(this->params.verticalScaleBarText);
        this->lineEditHorizontalScaleBarText->setText(this->params.horizontalScaleBarText);
        this->spinBoxHorizontalScaleBar->setValue(this->params.horizontalScaleBarLength);
        this->spinBoxVerticalScaleBar->setValue(this->params.verticalScaleBarLength);
    }
    this->updateDisplayFrameSettings();
    this->connectGuiToSettingsChangedSignal();
}

void ControlPanel2D::updateDisplayFrameSettings() {
    emit displayFrameSettingsChanged(this->spinBoxFrame->value(), this->spinBoxAverage->value(), this->stringBoxFunctions->getIndex());
}

void ControlPanel2D::toggleExtendedView() {
    this->enableExtendedView(!this->extendedView);
}
//_______________________
