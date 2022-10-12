#include "camera.h"

QList<QCameraInfo> Camera::infoList;
Camera::Camera(QObject *parent)
    : QObject(parent),
      device(nullptr),
      probe(nullptr),
      imagecapture(nullptr),
      recorder(nullptr),
      id(0)
{
    if (infoList.isEmpty()) {
        return;
    }
}

Camera::~Camera()
{
    stop();
}

void Camera::setProcess(std::function<void (const QVideoFrame &)> process_)
{
    process = process_;
    return;
}

void Camera::searchAndLock()
{
    device->searchAndLock();
    return;
}

void Camera::unlock()
{
    device->unlock();
    return;
}

void Camera::start(int devID)
{
    id = devID;
    /* select camera */
    device = new QCamera(infoList.at(devID));
    probe = new QVideoProbe(this);
    probe->setSource(device);
    connect(probe, &QVideoProbe::videoFrameProbed,
            this, &Camera::processFrame, Qt::DirectConnection);
    /* capture mode */
#ifdef Q_OS_ANDROID
    device->setCaptureMode(QCamera::CaptureVideo);
#else
    device->setCaptureMode(QCamera::CaptureStillImage);
#endif
    /* start camera */
    device->start();
    /* get parameters */
    imagecapture = new QCameraImageCapture(device);
    const QList<QSize> supportedResolutions = imagecapture->supportedResolutions();
    for (const QSize &res : supportedResolutions) {
        resolutions.append(res);
    }
    /* recorder */
#if 0
    recorder = new QMediaRecorder(device);
    connect(recorder, &QMediaRecorder::durationChanged, this, [=](){
        QString sec = QString("Recorded %1 sec").arg(recorder->duration()/1000);
        emit updateRecordTime(sec);
    });
    connect(recorder, QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &Camera::recordError);

    recorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/amr");
    audioSettings.setQuality(QMultimedia::HighQuality);
    recorder->setAudioSettings(audioSettings);
    QVideoEncoderSettings videoSettings;
    videoSettings.setCodec("video/mp4");
    videoSettings.setQuality(QMultimedia::VeryHighQuality);
    videoSettings.setBitRate(30);
    videoSettings.setResolution(w, h);
    recorder->setVideoSettings(videoSettings);
#endif
    /* resolution */
    QCameraViewfinderSettings settings;
#ifdef Q_OS_ANDROID
    settings.setPixelFormat(QVideoFrame::Format_NV21);
#else
    settings.setPixelFormat(QVideoFrame::Format_YUYV);
#endif
    settings.setResolution(QSize(w, h));
    device->setViewfinderSettings(settings);
    /* focus */
    device->searchAndLock();
    QTimer::singleShot(500, Qt::PreciseTimer, this, [=](){
        device->unlock();
    });
    return;
}

void Camera::restart(int devID)
{
    if (devID == id) {
        return;
    }
    stop();
    start(devID);
    return;
}

void Camera::processFrame(const QVideoFrame &frame)
{
    process(frame);
    return;
}

void Camera::stop()
{
    if (device == nullptr || probe == nullptr
            || imagecapture == nullptr || recorder == nullptr) {
        return;
    }
    device->stop();
    device->deleteLater();
    probe->deleteLater();
    imagecapture->deleteLater();
    recorder->deleteLater();
    device = nullptr;
    probe = nullptr;
    imagecapture = nullptr;
    recorder = nullptr;
    return;
}

void Camera::startRecord(const QString &videoPath)
{
    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
#ifdef Q_OS_ANDROID
    QString fileName = QString("%1/video_%2.mp4").arg(videoPath).arg(dateTime);
#else
    QString fileName = QString("./video_%2.mp4").arg(dateTime);
#endif
    recorder->setOutputLocation(QUrl::fromLocalFile(fileName));
    recorder->record();
    return;
}

void Camera::stopRecord()
{
    recorder->stop();
    return;
}

void Camera::pauseRecord()
{
    recorder->pause();
    return;
}

QImage Camera::imageFromVideoFrame(const QVideoFrame& buffer)
{
    QImage img;
    QVideoFrame frame(buffer);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(
                frame.pixelFormat());
    // BUT the frame.pixelFormat() is QVideoFrame::Format_Jpeg, and this is
    // mapped to QImage::Format_Invalid by
    // QVideoFrame::imageFormatFromPixelFormat
    if (imageFormat != QImage::Format_Invalid) {
        img = QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     // frame.bytesPerLine(),
                     imageFormat);
    } else {
        // e.g. JPEG
        int nbytes = frame.mappedBytes();
        img = QImage::fromData(frame.bits(), nbytes);
    }
    frame.unmap();
    return img;
}
