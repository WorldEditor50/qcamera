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
#include <QAction>
#include <QMenuBar>
#include <QVector>
#include <QDebug>
#include <QThread>
#include <functional>
#include "logger.hpp"

class Camera : public QObject
{
    Q_OBJECT
public:
    constexpr static int w = 640;
    constexpr static int h = 480;
public:
    explicit Camera(QObject *parent = nullptr);
    ~Camera();
    void setProcess(std::function<void(const QVideoFrame &)> process_);
    void searchAndLock();
    void unlock();
signals:
    void recordError();
    void updateRecordTime(const QString &sec);
public slots:
    void start(int devID);
    void restart(int devID);
    void stop();
    void startRecord(const QString &videoPath);
    void stopRecord();
    void pauseRecord();
private slots:
    void processFrame(const QVideoFrame &frame);
public:
    static QList<QCameraInfo> infoList;
    QVector<QSize> resolutions;
private:
    QCamera *device;
    QVideoProbe *probe;
    QCameraImageCapture *imagecapture;
    QMediaRecorder *recorder;
    QThread recordThread;
    std::function<void(const QVideoFrame&)> process;
    int id;
};

#endif // CAMERA_H
