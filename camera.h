#ifndef CAMERA_H
#define CAMERA_H
#include <QObject>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QFile>
#include <QDir>
#include <QVideoProbe>
#include <QMediaRecorder>
#include <QMediaMetaData>
#include <QDateTime>
#include <QUrl>
#include <QVector>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <functional>
#include "logger.hpp"

class Camera : public QObject
{
    Q_OBJECT
public:
    using FnProcess = std::function<void(const QVideoFrame &)>;
public:
    explicit Camera(QObject *parent = nullptr);
    ~Camera();
    void setProcess(const FnProcess &process_);
    void searchAndLock();
    void unlock();
public slots:
    void start(int id, int w, int h);
    void restart(int id);
    void resize(int w, int h);
    void stop();
private slots:
    void processFrame(const QVideoFrame &frame);
public:
    static QList<QCameraInfo> infoList;
    QVector<QSize> resolutions;
    int width;
    int height;
private:
    int cameraID;
    QCamera *device;
    QVideoProbe *probe;
    QCameraImageCapture *imagecapture;
    std::function<void(const QVideoFrame&)> process;
};

#endif // CAMERA_H
