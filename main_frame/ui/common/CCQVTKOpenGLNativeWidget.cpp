#include "CCQVTKOpenGLNativeWidget.h"
#include <QOpenGLContext>

CCQVTKOpenGLNativeWidget::CCQVTKOpenGLNativeWidget(QWidget *parent, Qt::WindowFlags f):
    QVTKOpenGLNativeWidget(parent, f)
{

}

CCQVTKOpenGLNativeWidget::~CCQVTKOpenGLNativeWidget()
{

}

void CCQVTKOpenGLNativeWidget::createOpenGLContextForProcesing(QOpenGLContext *processingContext, QOffscreenSurface *processingSurface, QThread *processingThread)
{
    QOpenGLContext* renderContext = this->context();
//    if(!renderContext) return;
    renderContext->create();
    (processingContext)->setFormat(renderContext->format());
    (processingContext)->setShareContext(renderContext);
    (processingContext)->create();
    (processingContext)->moveToThread(processingThread);
    (processingSurface)->setFormat(renderContext->format());
    (processingSurface)->create();
    (processingSurface)->moveToThread(processingThread);
}
