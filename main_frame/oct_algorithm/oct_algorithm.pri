include(cuda/cuda.pri)

HEADERS += \
    $$PWD/CCLoadSystem.h \
    $$PWD/CCSystemManager.h \
    $$PWD/oct_params/CCSystemSettings.h \
    $$PWD/oct_params/octalgorithmparameters.h \
    $$PWD/oct_params/windowfunction.h \
    $$PWD/oct_params/polynomial.h \
    $$PWD/oct_params/settings.h \
    $$PWD/processing.h \
    $$PWD/glfunctions/mesh.h \
    $$PWD/glfunctions/raycastvolume.h \
    $$PWD/gpu2hostnotifier.h \
    $$PWD/oct_params/recorder.h \
    $$PWD/CCVolumeCollector.h \
    $$PWD/oct_params/Algorithm.Grpc.h \
    $$PWD/GrpcAlgoProxy.h

SOURCES += \
    $$PWD/CCLoadSystem.cpp \
    $$PWD/CCSystemManager.cpp \
    $$PWD/oct_params/CCSystemSettings.cpp \
    $$PWD/oct_params/octalgorithmparameters.cpp \
    $$PWD/oct_params/windowfunction.cpp \
    $$PWD/oct_params/polynomial.cpp \
    $$PWD/oct_params/settings.cpp \
    $$PWD/processing.cpp \
    $$PWD/glfunctions/mesh.cpp \
    $$PWD/glfunctions/raycastvolume.cpp \
    $$PWD/gpu2hostnotifier.cpp \
    $$PWD/oct_params/recorder.cpp \
    $$PWD/CCVolumeCollector.cpp \
    $$PWD/GrpcAlgoProxy.cpp
