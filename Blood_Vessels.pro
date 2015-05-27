#-------------------------------------------------
#
# Project created by QtCreator 2015-02-14T16:46:27
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Blood_Vessels
TEMPLATE = app

CONFIG += static use_static_libs


SOURCES += main.cpp\
        mainwindow.cpp \
    image.cpp \
    edge.cpp \
    bloodvessels.cpp \
    slideshow.cpp

HEADERS  += mainwindow.h \
    image.h \
    edge.h \
    bloodvessels.h \
    slideshow.h

FORMS    += mainwindow.ui \
    edge.ui \
    bloodvessels.ui \
    slideshow.ui

DISTFILES += \
    README.md


use_static_libs {   #Use this for static release/deployment
    INCLUDEPATH += C:\\Programs\\opencvMinGW6\\install\\include

    LIBS += -LC:\\Programs\\opencvMinGW6\\install\\x64\\mingw\\staticlib \
                -lopencv_videoio300 \
        -lopencv_imgproc300 \
        -lopencv_imgcodecs300 \
        -lopencv_highgui300 \
        -lopencv_core300 \
                -ljpeg \
        -lIlmImf \
        -lzlib

    #Below two needed to resolve library errors
    LIBS += -LC:\\Qt\\Tools\\mingw491_32\\i686-w64-mingw32\\lib \
        -lcomctl32 \ #provides reference to CreateToolbarEx
        -lvfw32 #provides reference to AVIStreamRelease (creates .avi)

} else {   #Use this for debug mode
    INCLUDEPATH += C:\\Programs\\opencvMinGW\\install\\include

    LIBS += -LC:\\Programs\\opencvMinGW\\install\\x64\\mingw\\bin \
        -lopencv_core300 \
        -lopencv_highgui300 \
        -lopencv_imgcodecs300 \
        -lopencv_imgproc300 \
                -lopencv_videoio300
}
