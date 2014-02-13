#-------------------------------------------------
#
# Project created by QtCreator 2014-01-29T17:17:54
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = exmage-viewer
TEMPLATE = app
LIBS += -lGLU -lpng -lz

SOURCES += main.cpp\
        mainwindow.cpp \
    viewer.cpp \
    APNGReader.cpp \
    vector.cpp \
    quaternion.cpp \
    point.cpp \
    PNGWriter.cpp \
    Frame.cpp \
    PNGReader.cpp \
    CameraCore.cpp \
    TF.cpp \
    TFEditor.cpp

HEADERS  += mainwindow.h \
    viewer.h \
    APNGReader.h \
    CameraCore.h \
    vector.h \
    quaternion.h \
    point.h \
    PNGWriter.h \
    Frame.h \
    PNGReader.h \
    Vector.h \
    TF.h \
    TFEditor.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    shaders/pointcloud.vert \
    shaders/pointcloud.frag
