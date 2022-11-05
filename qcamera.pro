QT       += core gui multimedia multimediawidgets network
QT       += concurrent sql opengl androidextras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camera.cpp \
    cascade.cpp \
    configuration.cpp \
    imageprocess.cpp \
    imageviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    pipeline.cpp \
    setting.cpp \
    yolo.cpp \
    yolov4.cpp \
    yolov5.cpp \
    yolov7.cpp

HEADERS += \
    camera.h \
    cascade.h \
    configuration.h \
    imageprocess.h \
    imageviewer.h \
    mainwindow.h \
    openglwidget.h \
    pipeline.h \
    setting.h \
    util.h \
    yolo.h \
    yolov4.h \
    yolov5.h \
    yolov7.h

FORMS += \
    imageviewer.ui \
    mainwindow.ui \
    setting.ui
include($$PWD/shared/shared.pri)
include($$PWD/ffmpegutil/ffmpegutil.pri)
contains(ANDROID_TARGET_ARCH, arm64-v8a) {
    # libyuv
    INCLUDEPATH += $$PWD/libyuv/include
    LIBS += -L$$PWD/libyuv/lib/arm64-v8a -lyuv
    # opencv
    OPENCV_PATH = /home/eigen/Downloads/opencv-4.5.5-android-sdk/OpenCV-android-sdk/sdk/native
    INCLUDEPATH += $$OPENCV_PATH/jni/include
    LIBS += -L$$OPENCV_PATH/libs/arm64-v8a -lopencv_java4
    # ncnn
    NCNN_PATH = $$PWD/ncnn/arm64-v8a
    INCLUDEPATH += $$NCNN_PATH/include/ncnn
    LIBS += -L$$NCNN_PATH/lib -lncnn
    # ffmpeg
    FFMPEG_PATH = /home/eigen/MySpace/MyGithub/qcamera/ffmpeg/arm64-v8a
    INCLUDEPATH += $$FFMPEG_PATH/include
    LIBS += -L$$FFMPEG_PATH -lffmpeg-debug

} else {
    #libyuv
    LIBYUV_PATH = /home/eigen/MySpace/3rdPartyLibrary/libyuv
    INCLUDEPATH += $$LIBYUV_PATH/include
    LIBS += -L$$LIBYUV_PATH/lib -lyuv -ljpeg
    #opencv
    OPENCV_PATH = /home/eigen/MySpace/3rdPartyLibrary/opencv452
    INCLUDEPATH += $$OPENCV_PATH/include/opencv4
    LIBS += -L$$OPENCV_PATH/lib -lopencv_calib3d \
                                -lopencv_core \
                                -lopencv_dnn \
                                -lopencv_features2d \
                                -lopencv_flann \
                                -lopencv_gapi \
                                -lopencv_highgui \
                                -lopencv_imgcodecs \
                                -lopencv_imgproc \
                                -lopencv_ml \
                                -lopencv_objdetect \
                                -lopencv_photo \
                                -lopencv_stitching \
                                -lopencv_video \
                                -lopencv_videoio
    # ncnn
    NCNN_PATH = /home/eigen/MySpace/3rdPartyLibrary/ncnn
    INCLUDEPATH += $$NCNN_PATH/include/ncnn
    LIBS += -L$$NCNN_PATH/lib -lncnn
    # ffmpeg
    FFMPEG_PATH = /home/eigen/MySpace/3rdPartyLibrary/ffmpeg-gpl-shared
    INCLUDEPATH += $$FFMPEG_PATH/include
    LIBS += -L$$FFMPEG_PATH/lib -lavdevice \
                                -lavfilter \
                                -lavformat \
                                -lavcodec \
                                -lswscale \
                                -lswresample \
                                -lavutil \
                                -lpostproc
}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

RESOURCES += \
    models.qrc
