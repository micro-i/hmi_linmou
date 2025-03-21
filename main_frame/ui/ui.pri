FORMS += \
    $$PWD/CCShowForm.ui \
    $$PWD/MainWidget.ui \
    $$PWD/running_frame/CCProductManageDialog.ui \
    $$PWD/running_frame/CCSystemInfoForm.ui \
    $$PWD/running_frame/CCPositionViewForm.ui \
    $$PWD/running_frame/CCCheckoutResultForm.ui \
    $$PWD/running_frame/CCDefectsStatisticsForm.ui \
    $$PWD/running_frame/CCRunningCombineForm.ui \
    $$PWD/running_frame/CCSwitchRoleDialog.ui \
    $$PWD/CCSettingDialog.ui \
    $$PWD/common/CCPublicTitleBar.ui \
    $$PWD/running_frame/CCChoseProductDialog.ui \
    $$PWD/workpiece_frame/CCControlPanel.ui \
    $$PWD/workpiece_frame/CCVtkDialog3d.ui

HEADERS += \
    $$PWD/CCSettingDialog.h \
    $$PWD/CCShowForm.h \
    $$PWD/MainWidget.h \
    $$PWD/common/CCQVTKOpenGLNativeWidget.h \
    $$PWD/common/CCRadiusDialog.h \
    $$PWD/running_frame/CCPositionViewForm.h \
    $$PWD/running_frame/CCCheckoutResultForm.h \
    $$PWD/running_frame/CCDefectsStatisticsForm.h \
    $$PWD/running_frame/CCProductManageDialog.h \
    $$PWD/running_frame/CCRunningCombineForm.h \
    $$PWD/running_frame/CCSwitchRoleDialog.h \
    $$PWD/running_frame/CCSystemInfoForm.h \
    $$PWD/common/CCSwitchButton.h \
    $$PWD/statistic_frame/CCStatisticFrame.h \
    $$PWD/statistic_frame/CCTabWidget.h \
    $$PWD/workpiece_frame/CCControlPanel.h \
    $$PWD/workpiece_frame/CCWorkpiece2d.h \
    $$PWD/common/CCPublicTitleBar.h \
    $$PWD/running_frame/CCChoseProductDialog.h \
    $$PWD/workpiece_frame/CCWorkpiece3d.h   \
    $$PWD/workpiece_frame/CCVtkDialog3d.h

SOURCES += \
    $$PWD/CCSettingDialog.cpp \
    $$PWD/CCShowForm.cpp \
    $$PWD/MainWidget.cpp \
    $$PWD/common/CCQVTKOpenGLNativeWidget.cpp \
    $$PWD/common/CCRadiusDialog.cpp \
    $$PWD/running_frame/CCPositionViewForm.cpp \
    $$PWD/running_frame/CCCheckoutResultForm.cpp \
    $$PWD/running_frame/CCDefectsStatisticsForm.cpp \
    $$PWD/running_frame/CCProductManageDialog.cpp \
    $$PWD/running_frame/CCRunningCombineForm.cpp \
    $$PWD/running_frame/CCSwitchRoleDialog.cpp \
    $$PWD/running_frame/CCSystemInfoForm.cpp \
    $$PWD/common/CCSwitchButton.cpp \
    $$PWD/statistic_frame/CCStatisticFrame.cpp \
    $$PWD/statistic_frame/CCTabWidget.cpp \
    $$PWD/workpiece_frame/CCControlPanel.cpp \
    $$PWD/workpiece_frame/CCWorkpiece2d.cpp \
    $$PWD/common/CCPublicTitleBar.cpp \
    $$PWD/running_frame/CCChoseProductDialog.cpp \
    $$PWD/workpiece_frame/CCWorkpiece3d.cpp \
    $$PWD/workpiece_frame/CCVtkDialog3d.cpp

# vtk
CONFIG(debug, debug|release) {
        unix{
        }
        win32{
                INCLUDEPATH += C:/libraries/VTK-debug/include/vtk-8.2
                LIBS += C:/libraries/VTK-debug/lib/*.lib
                LIBS += -LC:/libraries/VTK-debug/bin/
        }
}

CONFIG(release, debug|release) {
        unix{
        }
        win32{
                INCLUDEPATH += C:/libraries/VTK/include/vtk-8.2
                LIBS += C:/libraries/VTK/lib/*.lib
                LIBS += -LC:/libraries/VTK/bin/
        }
}

RESOURCES += \
    $$PWD/ui.qrc

DISTFILES +=
