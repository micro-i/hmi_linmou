#include "CCWorkpiece2d.h"

#include <QTimer>
#include <QDebug>
#include <QMutex>
#include <QPainter>
#include <QCoreApplication>
#include <QtConcurrent>

#include "infrastructure/OCTGlobal.h"
#include "infrastructure/CCSignalBridge.h"


CCWorkpiece2d::CCWorkpiece2d(int screenNum, QWidget* parent) : QOpenGLWidget(parent),
    m_screenNum(screenNum)
{
    // 显示区域大小
    this->width = DEFAULT_WIDTH;
    this->height = DEFAULT_HEIGHT;
    this->depth = 0;    // 给3d用
    // gl设置
    this->initialized = false;
    this->changeBufferSizeFlag = false;
    this->delayedUpdatingRunning = false;
    // 允许手动刷新
    this->setUpdatesEnabled(true);
    // 界面显示信息：是否有缺陷、工件号、块号
    m_haveDefect.store(0);
    m_workPieceID.store(0);
    m_blockID.store(0);
    connect(this, &CCWorkpiece2d::addData, &m_vtk3dDialog, &CCVtkDialog3d::addData, Qt::QueuedConnection);
    connect(this, &CCWorkpiece2d::addDefects, &m_vtk3dDialog, &CCVtkDialog3d::addDefects, Qt::QueuedConnection);
}

CCWorkpiece2d::~CCWorkpiece2d()
{
    if(m_data){
        free(m_data);
    }
    if(m_currentShowData){
        free(m_currentShowData);
    }
    glDeleteBuffers(1, &(this->buf));
    glDeleteTextures(1, &(this->texture));
}

void CCWorkpiece2d::initSize(const unsigned int twidth, const unsigned int theight, const unsigned int tdepth){
    this->width = twidth;
    this->height = theight;
    this->depth = tdepth;
    m_haveDefect.store(0);
}

void CCWorkpiece2d::updateScreenInfo(const quint32 workPieceID, const quint32 blockID)
{
    m_workPieceID.store(workPieceID);
    m_blockID.store(blockID);
    // 重置是否有缺陷，还可以用来判断buffer是否显示完全
    m_haveDefect.store(0);
     m_ready3d.store(false);
}

void CCWorkpiece2d::updateDefect(int defectNum)
{
    m_haveDefect.store(defectNum);
    m_ready3d.store(true);
}

bool CCWorkpiece2d::compareScreen(const quint32 workPieceID, const quint32 blockID)
{
    return (m_workPieceID.load() == workPieceID) && (m_blockID.load() == blockID);
}

void CCWorkpiece2d::setData(void *data, int bitNum)
{
    m_3dlk.lock();
    if(m_data)
    free(m_data);
    m_data = data;
    m_bitNum = bitNum;
    m_3dlk.unlock();

    QtConcurrent::run([&]{
        QThread::sleep(150);
        QString filename = QString("vrLens-%1.raw").arg(m_blockID);
        QFile outputFile(filename);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            return;
        }
        outputFile.write((char*)m_data, 1024000000);
        outputFile.close();
    });
}

void CCWorkpiece2d::setDefects(const QList<QVector<float>>& defects)
{
    m_defects = defects;
}

void CCWorkpiece2d::delayedUpdate() {
    this->update();
    this->delayedUpdatingRunning = false;
}

void CCWorkpiece2d::changeTextureSize(unsigned int twidth, unsigned int theight, unsigned int tdepth) {
    this->width = twidth <= 1 ? 128 : twidth; //width and height shall not be zero. 128 is an arbitrary value greater 1 and a power of 2
    this->height = theight <= 1 ? 128 : theight;
    this->depth = tdepth;

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
    emit registerRunBufferCudaGL(this->buf, m_screenNum, false);
    // 更新GL屏幕大小
    auto size = this->size();
    this->resize(this->width, this->height);
    this->resize(size);
}
void CCWorkpiece2d::createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread) {

    QOpenGLContext* renderContext = this->context();
    renderContext->create();
    QMutexLocker lck(&OCTGlobal::glContextMtx);
    (processingContext)->setFormat(renderContext->format());
    (processingContext)->setShareContext(renderContext);
    (processingContext)->create();
    (processingContext)->moveToThread(processingThread);
    (processingSurface)->setFormat(renderContext->format());
    (processingSurface)->create();
    //Due to the fact that QOffscreenSurface is backed by a QWindow on some platforms, cross-platform applications must ensure that create() is only called on the main (GUI) thread
    (processingSurface)->moveToThread(processingThread);
    // 更新并注册
    this->changeTextureSize(this->width, this->height, this->depth);

}

void CCWorkpiece2d::registerOpenGLBufferWithCuda() {
    emit registerRunBufferCudaGL(this->buf, m_screenNum, false);
}

void CCWorkpiece2d::initializeGL() {
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
        emit registerRunBufferCudaGL(this->buf, m_screenNum, false); //registerBufferCudaGL is necessary here because as soon as the openglwidget/dock is removed from the main window initializeGL() is called again.
    }

    this->initialized = true;

    if(this->changeBufferSizeFlag){
        this->changeTextureSize(this->width, this->height, this->depth);
        this->changeBufferSizeFlag = false;
    }
}

void CCWorkpiece2d::displayMarker() {
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
    if(m_haveDefect.load() == -1){ // 有缺陷
        glColor3f(0.8f, 0.0f, 0.0f);
    }else if(m_haveDefect.load() == 1){ // 无缺陷
        glColor3f(0.0f, 0.8f, 0.0f);
    }else{// 未存储
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    glVertex3f(-1.0, -1.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glEnd();
    QString info = QString(u8"工件号: %1; 块号: %2").arg(m_workPieceID.load()).arg(m_blockID.load());
    QFont font;
    font.setPointSize(10);

    QPainter painter;
    painter.begin(this);
    painter.setFont(this->font());
    int tempjudge = m_haveDefect.load();
    painter.setPen(tempjudge == 1?QColor(0, 204, 0):
                                  tempjudge == 0?QColor(127, 127, 127) :
                                                 QColor(204, 0, 0));
    painter.drawText(QPoint(10, 30), info);
    painter.end();
//    renderText(-0.9f, 0.75f, 0, info.toLatin1(), font);
}

void CCWorkpiece2d::renderText(double x, double y, double z, const QString &text, const QFont& font)
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
    if(m_haveDefect.load() < 0){
        painter.setPen(Qt::red);
    }else{
        painter.setPen(Qt::green);
    }
    painter.setFont(font);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(textPosX, textPosY, text); // z = pointT4.z + distOverOp / 4
    painter.end();
}

inline GLint CCWorkpiece2d::project(GLdouble objx, GLdouble objy, GLdouble objz,
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

inline void CCWorkpiece2d::transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
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

void CCWorkpiece2d::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除屏幕缓存和深度缓冲
    //reset matrix state (resets previous translation, rotation and scale operations)
    glLoadIdentity();
    //display oct data
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, this->buf);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    if(m_reverse){
        glTexCoord2f(1, 0), glVertex2f(-1.0f, -1.0f); //place upper left texture coordinate to bottom left screen position
        glTexCoord2f(0, 0), glVertex2f(-1.0f, 1.0f); //upper right texture coordinate to upper left screen position
        glTexCoord2f(0, 1), glVertex2f(1.0f, 1.0f); //bottom right texture coordinate to upper right screen position
        glTexCoord2f(1, 1), glVertex2f(1.0f, -1.0f); //bottom left texture coordinate to bottom right screen position
    }else{
        glTexCoord2f(0, 1), glVertex2f(-1.0f, -1.0f); //place upper left texture coordinate to bottom left screen position
        glTexCoord2f(1, 1), glVertex2f(-1.0f, 1.0f); //upper right texture coordinate to upper left screen position
        glTexCoord2f(1, 0), glVertex2f(1.0f, 1.0f); //bottom right texture coordinate to upper right screen position
        glTexCoord2f(0, 0), glVertex2f(1.0f, -1.0f); //bottom left texture coordinate to bottom right screen position
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    // 显示Mark图
    this->displayMarker();

    if(!this->delayedUpdatingRunning){
        this->delayedUpdatingRunning = true;
        QTimer::singleShot(DELAY_TIME_IN_ms, this, QOverload<>::of(&CCWorkpiece2d::delayedUpdate));
    }
}

void CCWorkpiece2d::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    if(!m_ready3d.load()) return;
    m_3dlk.lock();
    if(!m_data && m_currentShowData){
       m_3dlk.unlock();
       emit CCSignalBridge::instance()->setMask(true);
       m_vtk3dDialog.show();
       return;
    }
    if(!m_data){
        m_3dlk.unlock();
        return;
    }
    if(m_currentShowData)
    free(m_currentShowData);
    m_currentShowData = m_data;
    m_data = nullptr;
    m_3dlk.unlock();
//    m_vtk3dDialog.setWindowModality(Qt::ApplicationModal);
    m_vtk3dDialog.setModal(true);
    m_vtk3dDialog.setTitle(QString(u8"缺陷详情,工件号：%1,块号：%2").arg(m_workPieceID.load()).arg(m_blockID.load()));
    emit CCSignalBridge::instance()->setMask(true);
    m_vtk3dDialog.show();
    emit addData(m_currentShowData, depth, width, height, m_bitNum);
    for(auto& defect : m_defects){
        if(defect.size() < 6){
            qWarning() << "3d defect position error!";
            continue;
        }
        emit addDefects(defect[0],defect[1],defect[2],defect[3],defect[4],defect[5], QColor(255, 0, 0));
    }
}
