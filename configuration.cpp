#include "configuration.h"
#include <opencv2/imgproc.hpp>

void Configuration::load()
{
    /* yolov5 */
    QString yolov5Param = QString("%1/yolov5s_6.0.param").arg(modelPath);
    if (QFile(yolov5Param).exists() == false) {
        QFile::copy(":/yolo/ncnn/models/yolov5s_6.0.param", yolov5Param);
        QFile::setPermissions(yolov5Param, QFileDevice::ReadOther);
    }
    QString yolov5Model = QString("%1/yolov5s_6.0.bin").arg(modelPath);
    if (QFile(yolov5Model).exists() == false) {
        QFile::copy(":/yolo/ncnn/models/yolov5s_6.0.bin", yolov5Model);
        QFile::setPermissions(yolov5Model, QFileDevice::ReadOther);
    }

    /* yolov7 */
    QString yolov7Param = QString("%1/yolov7-tiny.param").arg(modelPath);
    if (QFile(yolov7Param).exists() == false) {
        QFile::copy(":/yolo/ncnn/models/yolov7-tiny.param", yolov7Param);
        QFile::setPermissions(yolov7Param, QFileDevice::ReadOther);
    }
    QString yolov7Model = QString("%1/yolov7-tiny.bin").arg(modelPath);
    if (QFile(yolov7Model).exists() == false) {
        QFile::copy(":/yolo/ncnn/models/yolov7-tiny.bin", yolov7Model);
        QFile::setPermissions(yolov7Model, QFileDevice::ReadOther);
    }
    /* haarcascade */
    QString haarcascadeEye = QString("%1/haarcascade_eye_tree_eyeglasses.xml").arg(modelPath);
    if (QFile(haarcascadeEye).exists() == false) {
        QFile::copy(":/haarcascade/data/haarcascade_eye_tree_eyeglasses.xml", haarcascadeEye);
        QFile::setPermissions(haarcascadeEye, QFileDevice::ReadOther);
    }
    QString haarcascadeFace = QString("%1/haarcascade_frontalface_alt.xml").arg(modelPath);
    if (QFile(haarcascadeFace).exists() == false) {
        QFile::copy(":/haarcascade/data/haarcascade_frontalface_alt.xml", haarcascadeFace);
        QFile::setPermissions(haarcascadeFace, QFileDevice::ReadOther);
    }
    QString haarcascadeUpperbody = QString("%1/haarcascade_upperbody.xml").arg(modelPath);
    if (QFile(haarcascadeUpperbody).exists() == false) {
        QFile::copy(":/haarcascade/data/haarcascade_upperbody.xml", haarcascadeUpperbody);
        QFile::setPermissions(haarcascadeUpperbody, QFileDevice::ReadOther);
    }
    /* video format */
    videoFormat = "mp4";
    /* capture style */
    captureStyle = "none";
    return;
}

Configuration::Configuration()
{
    isRoateFrame = true;
    storagePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    /* app data */
    appDataPath = QString("%1/qcamera").arg(storagePath);
    QDir appDataDir(appDataPath);
    if (appDataDir.exists() == false) {
        appDataDir.mkdir(appDataPath);
    }
    /* model */
    modelPath = QString("%1/models").arg(appDataPath);
    QDir modelDir(modelPath);
    if (modelDir.exists() == false) {
        modelDir.mkdir(modelPath);
    }
    /* picture */
    picturePath = QString("%1/picture").arg(appDataPath);
    QDir pictureDir(picturePath);
    if (pictureDir.exists() == false) {
        pictureDir.mkdir(picturePath);
    }
    /* video */
    videoPath = QString("%1/video").arg(appDataPath);
    QDir videoDir(videoPath);
    if (videoDir.exists() == false) {
        videoDir.mkdir(videoPath);
    }
    return;
}
