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

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib \
         -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videoio -lopencv_videostab


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
