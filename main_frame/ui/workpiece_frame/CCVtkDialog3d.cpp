#include "CCVtkDialog3d.h"
#include "ui_CCVtkDialog3d.h"

// window
#include <QVTKOpenGLNativeWidget.h>
//#include <vtkOpenGLRenderWindow.h>
#include <vtkWin32OpenGLRenderWindow.h>
// Functional
#include <vtkOutputWindow.h>
// Interactor
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
// Coordinate
#include <vtkAxesActor.h>
// Actor
#include <vtkPointData.h>
#include <vtkLine.h>
#include <vtkDataSetMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVolumeMapper.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
// Transform
#include <vtkPiecewiseFunction.h>
// View
#include <vtkCamera.h>
// cutter
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkImageMapper.h>
#include <vtkImageActor.h>
#include <vtkImageSliceMapper.h>

#include <QGridLayout>

#include <QDebug>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QtConcurrent>

#include "infrastructure/CCSignalBridge.h"
#include "CCControlPanel.h"
#include "ui/common/CCQVTKOpenGLNativeWidget.h"

//#define VTK9

CCVtkDialog3d::CCVtkDialog3d(QWidget *parent) :
    CCRadiusDialog(parent),
    ui(new Ui::CCVtkDialog3d)
{
    //    vtkOutputWindow::SetGlobalWarningDisplay(0);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->setupUi(this);
    m_vtkWidget = new CCQVTKOpenGLNativeWidget(this);
    m_context = new QOpenGLContext();
    m_surface = new QOffscreenSurface();
    ui->gridLayout->addWidget(m_vtkWidget);
    // 切片
    m_panel = new CCControlPanel(this);
    m_panel->setMaximumHeight(50);
    m_panel->setAutoFillBackground(true);
    ui->gridLayout->addWidget(m_panel, 1, 0);
    initVTK();
    connect(m_panel, &CCControlPanel::enableCutter, this, [&](bool enable){
        m_sliceActor->SetVisibility(enable);
        if(enable)
            this->updateCutter(m_direction, m_frameNr);
        else{
#ifdef VTK9
            m_vtkWidget->renderWindow()->Render();
#else
            m_vtkWidget->GetRenderWindow()->Render();
#endif
        }
    });
    connect(m_panel, &CCControlPanel::disable3D, this, [&](bool enable){
        m_volume->SetVisibility(!enable);
#ifdef VTK9
        m_vtkWidget->renderWindow()->Render();
#else
        m_vtkWidget->GetRenderWindow()->Render();
#endif
    });
    connect(m_panel, &CCControlPanel::directionChanged, this, [&](int direction){
        this->updateCutter(direction, m_frameNr);
    });
    connect(m_panel, &CCControlPanel::frameChanged, this, [&](int frameNr){
        this->updateCutter(m_direction, frameNr);
    });
}

CCVtkDialog3d::~CCVtkDialog3d()
{
    delete ui;
    m_vtkThread.quit();
    m_vtkThread.wait();
    //    delete m_context;
    //    delete m_surface;
}


void CCVtkDialog3d::addData(void *data, int width, int height, int depth, int bitNum)
{
//    m_vtkWidget->createOpenGLContextForProcesing(m_context, m_surface, &m_vtkThread);
//    m_vtkThread.start();

    m_vtkRenderer->RemoveAllViewProps();
    // TODO 可能不一定是uchar*
    m_imageData = vtkSmartPointer<vtkStructuredPoints>::New();
    if(8 == bitNum){
        vtkSmartPointer<vtkUnsignedCharArray> charArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
        // auto delete
        charArray->SetArray((unsigned char*)data,  width*height*depth, 1);
        m_imageData->SetDimensions(width, height, depth);
        m_imageData->SetOrigin(0, 0, 0);
        m_imageData->SetSpacing(1, 1, 1);
        m_imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        m_imageData->GetPointData()->SetScalars(charArray);
    }else{
        return;
    }

    m_width = width;
    m_height = height;
    m_depth = depth;



    vtkSmartPointer<vtkPiecewiseFunction> gradient = vtkSmartPointer<vtkPiecewiseFunction>::New();
    gradient->AddPoint(0, 0);
    gradient->AddPoint(255, 1);

    vtkSmartPointer<vtkColorTransferFunction> colorScalar = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorScalar->AddRGBPoint(17.3149, 0.278431, 0.278431, 0.858824);
    colorScalar->AddRGBPoint(51.3039, 0, 0, 0.360784);
    colorScalar->AddRGBPoint(85.0552, 0, 1, 1);
    colorScalar->AddRGBPoint(119.282, 0, 0.501961, 0);
    colorScalar->AddRGBPoint(153.033, 1, 1, 0);
    colorScalar->AddRGBPoint(187.022, 1, 0.380392, 0);
    colorScalar->AddRGBPoint(221.011, 0.419608, 0, 0);
    colorScalar->AddRGBPoint(255, 0.878431, 0.301961, 0.301961);

    // VOLUME PROPERTY
    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->ShadeOff();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->SetScalarOpacity(gradient);
    volumeProperty->SetColor(colorScalar);

    vtkSmartPointer<vtkSmartVolumeMapper> hiresMapper =
            vtkSmartPointer<vtkSmartVolumeMapper>::New();
    hiresMapper->SetRequestedRenderModeToGPU();
    hiresMapper->SetBlendModeToComposite();
    hiresMapper->SetInputData(m_imageData);
    hiresMapper->Update();

//    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper =
//            vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
//    volumeMapper->SetInputData(m_imageData);
//    volumeMapper->SetAutoAdjustSampleDistances(1);
//    volumeMapper->Update();

    m_volume->SetMapper(hiresMapper);
    m_volume->SetProperty(volumeProperty);

    auto future = QtConcurrent::run([&]{
       m_volume->Update();
    });

    future.waitForFinished();
    m_vtkRenderer->AddVolume(m_volume);
    m_vtkRenderer->ResetCamera();
#ifdef VTK9
    m_vtkWidget->renderWindow()->Render();
#else
    m_vtkWidget->GetRenderWindow()->Render();
#endif
}

void CCVtkDialog3d::addDefects(const float minX, const float minY, const float minZ, const float maxX, const float maxY, const float maxZ, QColor color)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(8);
    points->SetPoint(0, minX, maxY, minZ);
    points->SetPoint(1, maxX, maxY, minZ);
    points->SetPoint(2, maxX, minY, minZ);
    points->SetPoint(3, minX, minY, minZ);

    points->SetPoint(4, minX, maxY, maxZ);
    points->SetPoint(5, maxX, maxY, maxZ);
    points->SetPoint(6, maxX, minY, maxZ);
    points->SetPoint(7, minX, minY, maxZ);

    vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
    for(int i = 0; i < 12; ++i)
    {
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        if(i < 4)
        {
            line->GetPointIds()->SetId(0,                                i);
            line->GetPointIds()->SetId(1, ((i + 1) % 4 != 0 ? (i + 1) : 0));
        }
        else if(i < 8)
        {
            line->GetPointIds()->SetId(0,                                i);
            line->GetPointIds()->SetId(1, ((i + 1) % 8 != 0 ? (i + 1) : 4));
        }
        else
        {
            line->GetPointIds()->SetId(0,     i % 4);
            line->GetPointIds()->SetId(1, i % 4 + 4);
        }
        cellArray->InsertNextCell(line);
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(cellArray);
    vtkSmartPointer<vtkDataSetMapper> mapper =vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(polyData);

    auto defectActor = vtkSmartPointer<vtkActor>::New();
    defectActor->SetMapper(mapper);
    defectActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    m_vtkRenderer->AddActor(defectActor);
#ifdef VTK9
    m_vtkWidget->renderWindow()->Render();
#else
    m_vtkWidget->GetRenderWindow()->Render();
#endif
}

void CCVtkDialog3d::setTitle(QString title)
{
    ui->widget->setTitle(title);
}

void CCVtkDialog3d::updateCutter(int direction, int frame)
{
    m_direction = direction;
    m_frameNr = frame;
    m_panel->setMaxIndex((0 == direction) ? m_width : ((1 == direction) ? m_height : m_depth));

    if(!m_sliceActor->GetVisibility()) return;

    auto mapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    mapper->SetInputData(m_imageData);
    mapper->SetSliceNumber(frame);
    if(0 == direction){
        mapper->SetOrientationToX();
    }else if(1 == direction){
        mapper->SetOrientationToY();
    }else{
        mapper->SetOrientationToZ();
    }

    m_sliceActor->SetMapper(mapper);
    m_vtkRenderer->AddActor(m_sliceActor);
#ifdef VTK9
    m_vtkWidget->renderWindow()->Render();
#else
    m_vtkWidget->GetRenderWindow()->Render();
#endif
}

void CCVtkDialog3d::initVTK()
{
    // get vtk renderwindow
    m_vtkRenderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
    m_sliceActor = vtkSmartPointer<vtkImageActor>::New();
    m_vtkRenderer->SetUseFXAA(0);
    m_vtkRenderer->SetUseShadows(0);
//    m_vtkRenderer->GetActiveCamera()->SetClippingRange(1, 3000);
#ifdef VTK9
    auto renderWindow = m_vtkWidget->renderWindow();
#else
    auto renderWindow = m_vtkWidget->GetRenderWindow();
#endif
    // axesactor
    vtkNew<vtkAxesActor> axesActor;
    axesActor->SetPosition(0, 0, 0);
    axesActor->SetTotalLength(2, 2, 2);
    axesActor->SetShaftType(0);
    axesActor->SetCylinderRadius(0.02);
    // interactorstyle
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> iStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    iStyle->SetDefaultRenderer(m_vtkRenderer);
#ifdef VTK9
    m_vtkWidget->interactor()->SetInteractorStyle(iStyle);
#else
    m_vtkWidget->GetInteractor()->SetInteractorStyle(iStyle);
#endif
    // renderer
    m_vtkRenderer->SetBackground(1.0, 1.0, 1.0);
    m_vtkRenderer->SetBackground2(0.529, 0.8078, 0.92157);
    m_vtkRenderer->SetGradientBackground(true);

    renderWindow->AddRenderer(m_vtkRenderer);
    // vtkOrientationMarkerWidget
    m_orWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_orWidget->SetOrientationMarker(axesActor);
    m_orWidget->SetOutlineColor(0.93, 0.57, 0.13);
    m_orWidget->InteractiveOff();
#ifdef VTK9
    m_orWidget->SetInteractor(m_vtkWidget->interactor());
#else
    m_orWidget->SetInteractor(m_vtkWidget->GetInteractor());
#endif
    m_orWidget->EnabledOn();
    m_volume = vtkSmartPointer<vtkVolume>::New();
    m_vtkRenderer->AddVolume(m_volume);
    m_vtkRenderer->ResetCamera();
    // render
    renderWindow->Render();
}
