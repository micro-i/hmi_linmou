#ifndef CCVTKDIALOG3D_H
#define CCVTKDIALOG3D_H

#include "ui/common/CCRadiusDialog.h"
#include <QThread>

#ifndef INITIAL_OPENGL
#define INITIAL_OPENGL
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkInteractionStyle)
#endif

#include <vtkOpenGLRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkLODProp3D.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkMultiThreader.h>
#include <vtkStructuredPoints.h>

namespace Ui {
class CCVtkDialog3d;
}

class CCVtkDialog3d : public CCRadiusDialog
{
    Q_OBJECT

public:
    explicit CCVtkDialog3d(QWidget *parent = 0);
    ~CCVtkDialog3d();
    // 添加数据
    void addData(void* data, int width, int height, int depth, int bitNum);
    // 添加缺陷
    void addDefects(const float xmin, const float ymin, const float zmin,
                    const float xmax, const float ymax, const float zmax, QColor color);
    // 设置界面标题
    void setTitle(QString title);
    // TODO 添加切片
    void updateCutter(int direction, int frame);

private:
    // 初始化VTK
    void initVTK();

    Ui::CCVtkDialog3d *ui;
    QOpenGLContext*                                 m_context;            // OpenGL上下文
    class QOffscreenSurface*                        m_surface;         // 用于多线程环境的OpenGL渲染
    QThread                                         m_vtkThread;
    class CCQVTKOpenGLNativeWidget*                 m_vtkWidget;
    class CCControlPanel*                           m_panel;
    vtkSmartPointer<vtkOpenGLRenderer>              m_vtkRenderer;          // render
    vtkSmartPointer<vtkOrientationMarkerWidget>     m_orWidget;             // Coordinate widget
    vtkSmartPointer<vtkVolume>                      m_volume;               // stl
    vtkSmartPointer<vtkStructuredPoints>                   m_imageData;
    vtkSmartPointer<vtkImageActor>                  m_sliceActor;
    int                                             m_direction = 0;
    int                                             m_frameNr = 0;
    int                                             m_width = 0;
    int                                             m_height = 0;
    int                                             m_depth = 0;
};

#endif // CCVTKDIALOG3D_H
