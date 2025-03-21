HEADERS += \
        $$PWD/src/CCParamForm.h \
        $$PWD/src/controlpanel.h \
        $$PWD/src/eventguard.h \
        $$PWD/src/extensioneventfilter.h \
        $$PWD/src/extensionmanager.h \
        $$PWD/src/glwindow2d.h \
        $$PWD/src/glwindow3d.h \
        $$PWD/src/messageconsole.h \
        $$PWD/src/minicurveplot.h \
        $$PWD/src/octproz.h \
        $$PWD/src/outputwindow.h \
        $$PWD/src/plotwindow1d.h \
        $$PWD/src/sidebar.h \
        $$PWD/src/stringspinbox.h \
        $$PWD/src/systemchooser.h \
        $$PWD/src/trackball.h \
        $$PWD/qcustomplot/qcustomplot.h


SOURCES += \
        $$PWD/src/CCParamForm.cpp \
        $$PWD/src/controlpanel.cpp \
        $$PWD/src/eventguard.cpp \
        $$PWD/src/extensioneventfilter.cpp \
        $$PWD/src/extensionmanager.cpp \
        $$PWD/src/glwindow2d.cpp \
        $$PWD/src/glwindow3d.cpp \
        $$PWD/src/messageconsole.cpp \
        $$PWD/src/minicurveplot.cpp \
        $$PWD/src/octproz.cpp \
        $$PWD/src/plotwindow1d.cpp \
        $$PWD/src/sidebar.cpp \
        $$PWD/src/stringspinbox.cpp \
        $$PWD/src/systemchooser.cpp \
        $$PWD/src/trackball.cpp \
        $$PWD/qcustomplot/qcustomplot.cpp



FORMS += \
        $$PWD/src/CCParamForm.ui \
        $$PWD/src/octproz.ui \
        $$PWD/src/sidebar.ui

RESOURCES += \
        $$PWD/resources.qrc

# OpenGL
unix{
        LIBS += -lGL -lGLU -lX11 -lglut
}
win32{
        LIBS += -lopengl32 -lglu32
}

#include pri file to copy color lookup tables to build folder
include(luts/luts.pri)

#set application icon
#RC_ICONS = icons/OCTproZ_icon.ico
