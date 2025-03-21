/**
  * @author ld
  * @brief  创建多线程渲染的vtk窗口 <尝试>
  * @date   2023-02-14
  */

#ifndef CCQVTKOPENGLNATIVEWIDGET_H
#define CCQVTKOPENGLNATIVEWIDGET_H
#include <QVTKOpenGLNativeWidget.h>
#include <QOffscreenSurface>

// 此类保留，但实际测试过程中效果欠佳
class CCQVTKOpenGLNativeWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT
public:
    CCQVTKOpenGLNativeWidget(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~CCQVTKOpenGLNativeWidget() override;
    /**
     * @brief createOpenGLContextForProcesing   创建一个共享上下文
     * @param processingContext 共享上下文
     * @param processingSurface 离屏像素保存区域
     * @param processingThread  处理线程
     */
    void createOpenGLContextForProcesing(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
};

#endif // CCQVTKOPENGLNATIVEWIDGET_H
