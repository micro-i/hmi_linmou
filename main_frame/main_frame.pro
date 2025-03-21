#-------------------------------------------------
#
# Project created by QtCreator 2022-10-27T10:50:39
#
#-------------------------------------------------

QT       += core gui svg sql concurrent printsupport websockets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# appdir
unix{
    DESTDIR = /home/adt/micro-i/hmi_oct/
}
win32{
    CONFIG(debug, debug|release){
        DESTDIR = $$PWD/../hmi_oct_produce-debug/
    }else{
        DESTDIR = $$PWD/../hmi_oct_produce/
        DEFINES += QT_MESSAGELOGCONTEXT # release output information
    }
}

TARGET = hmi_oct # exe name
TEMPLATE = app
RC_ICONS = logoicon.ico


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# oct_algorithm
include(oct_algorithm/oct_algorithm.pri)
# ui
include(ui/ui.pri)
# oct_appearance
include(oct_forms/oct_forms.pri)
# oct_devkit
include(oct_devkit/oct_devkit.pri)
# infrastructure in hmi_v3
include(infrastructure/infrastructure.pri)
# db
include(db/db.pri)
# data_structure
include(data_structure/data_structure.pri)
# grpc
include(grpc/grpc.pri)
# acquisition system
include(acquisition_system/acquisition_system.pri)
# camera
include(camera/camera.pri)
# daq
include(daq/daq.pri)
# translation vector
include(translation_vector/translation_vector.pri)

SOURCES += \
        main.cpp
