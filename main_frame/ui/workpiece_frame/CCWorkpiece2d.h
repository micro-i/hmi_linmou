#ifndef CCWORKPIECE2D_H
#define CCWORKPIECE2D_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOffscreenSurface>
#include <QThread>

// CUDA Runtime, Interop includes
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

#include <QDialog>
#include <QMutex>
#include <qatomic.h>

#include "CCVtkDialog3d.h"

#define DEFAULT_WIDTH  512
#define DEFAULT_HEIGHT 512

#define DELAY_TIME_IN_ms 40

class CCWorkpiece2d : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    /**
     * @brief CCWorkPiece2D
     * @param screenNum 当前窗体序号
     * @param parent 父窗体
     */
    CCWorkpiece2d(int screenNum, QWidget* parent = nullptr);
    ~CCWorkpiece2d();
    /**
     * @brief 初始化大小
     */
    void initSize(const unsigned int width, const unsigned int height, const unsigned int depth);
    /**
     * @brief updateScreenInfo 更新窗体信息
     * @param workPieceID 工件号
     * @param blockID 块号
     */
    void updateScreenInfo(const quint32 workPieceID, const quint32 blockID);
    /**
     * @brief updateDefect 更新是否有缺陷
     * @param defectNum
     */
    void updateDefect(int defectNum);
    /**
     * @brief compareScreen 比较是否为当前屏幕
     * @param workPieceID
     * @param blockID
     * @return
     */
    bool compareScreen(const quint32 workPieceID, const quint32 blockID);
    /**
     * @brief setData 设置三维缓冲数据
     * @param data
     * @param bitNum
     */
    void setData(void* data, int bitNum);
    /**
     * @brief setDefects 添加缺陷
     */
    void setDefects(const QList<QVector<float>>&);

    void setReverse(bool reverse){m_reverse = reverse;}
private:
    unsigned int width;                 // 图片宽度
    unsigned int height;                // 图片高度
    unsigned int depth;                 // 图片深度

    bool initialized;                   // GL初始化标志
    bool changeBufferSizeFlag;          // GLBuffer更换标志
    bool delayedUpdatingRunning;        // 刷新标志

    int m_screenNum = 0;                // 当前屏幕号，注册buf时用
    GLuint buf;                         // bufferID
    GLuint texture;                     // 纹理

    std::atomic_int8_t  m_haveDefect;   // 是否有缺陷
    std::atomic_uint32_t m_workPieceID;  // 工件号
    std::atomic_uint32_t m_blockID;      // 块号
    std::atomic_bool    m_ready3d;      // 三维数据准备完全

    bool m_reverse = false;

    QMutex m_3dlk;                      // 三维数据锁
    void* m_data = nullptr;             // 三维数据块
    void* m_currentShowData = nullptr;            // 当前显示数据块
    int m_bitNum = 0;                   // 数据位数
    QList<QVector<float>> m_defects;      // 三维缺陷
    CCVtkDialog3d m_vtk3dDialog; // 三维显示dialog
private:
    /**
     * @brief delayedUpdate 刷新
     */
    void delayedUpdate();
    /**
     * @brief displayMarker 显示Mark图框
     */
    void displayMarker();
    /**
     * @brief renderText 渲染文字
     * @param x x坐标
     * @param y y坐标
     * @param z z坐标
     * @param str   文字
     * @param font  字体
     */
    void renderText(double x, double y, double z, const QString &str, const QFont & font = QFont());
    /**
     * @brief project 文字坐标转换
     */
    inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                         const GLdouble model[16], const GLdouble proj[16],
                         const GLint viewport[4], GLdouble * winx, GLdouble * winy, GLdouble * winz);
    /**
     * @brief transformPoint 文字矩阵变换
     */
    inline void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
protected:
    /**
     * @brief initializeGL 初始化GL
     */
    void initializeGL() override;
    /**
     * @brief paintGL 绘制GL
     */
    void paintGL() override;
    /**
     * @brief mouseDoubleClickEvent 鼠标双击事件
     * @param event
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;
public slots:
    // 改变纹理大小
    void changeTextureSize(unsigned int width, unsigned int height, unsigned int depth);
    // 多线程渲染环境准备
    void createOpenGLContextForProcessing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
    // 注册Buffer
    void registerOpenGLBufferWithCuda();
signals:
    // 注册Buffer信号
    void registerRunBufferCudaGL(unsigned int bufferId, int currScreenNum, bool is3d);
    // 添加数据
    void addData(void* data, int width, int height, int depth, int bitNum);
    // 添加缺陷
    void addDefects(const float xmin, const float ymin, const float zmin,
                    const float xmax, const float ymax, const float zmax, QColor color);
};

#endif // CCWORKPIECE2D_H
