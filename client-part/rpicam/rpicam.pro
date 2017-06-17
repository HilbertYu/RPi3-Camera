#-------------------------------------------------
#
# Project created by QtCreator 2017-06-17T16:10:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rpicam
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui


INCLUDEPATH += /usr/local/Cellar/opencv3/3.2.0/include/

LIBS += -L/usr/local/Cellar/opencv3/3.2.0/lib \
-lopencv_shape  \
-lopencv_stitching  \
-lopencv_objdetect  \
-lopencv_superres  \
-lopencv_videostab  \
-lopencv_calib3d  \
-lopencv_features2d  \
-lopencv_highgui  \
-lopencv_videoio  \
-lopencv_imgcodecs  \
-lopencv_video  \
-lopencv_photo  \
-lopencv_ml  \
-lopencv_imgproc  \
-lopencv_flann  \
-lopencv_core  \
 \
