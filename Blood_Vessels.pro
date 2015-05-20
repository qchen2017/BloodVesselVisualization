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

INCLUDEPATH += C:\\OpenCV3.1\\myopencv\\install\\include
LIBS += -LC:\\OpenCV3.1\\myopencv\\install\\x64\\mingw\\bin \
        -lopencv_core300 \
        -lopencv_highgui300 \
        -lopencv_imgcodecs300 \
        -lopencv_imgproc300 \
        -lopencv_videoio300


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
    README.md \
    TEST CASES.md \
    TEST CASES.md
