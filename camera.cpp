#include "camera.h"

QList<QCameraInfo> Camera::infoList;
Camera::Camera(QObject *parent)
    : QObject(parent),
      width(0),
      height(0),
      cameraID(-1),
      device(nullptr),
      probe(nullptr),
      imagecapture(nullptr)
{
    Camera::infoList = QCameraInfo::availableCameras();
}

Camera::~Camera()
{
    stop();
}

void Camera::setProcess(const Camera::FnProcess &process_)
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

void Camera::start(int id, int w, int h)
{
    cameraID = id;
    width = w;
    height = h;
    Configuration::instance().setCameraParam(id, w, h);
    /* select camera */
    device = new QCamera(infoList.at(cameraID));
    probe = new QVideoProbe;
    probe->setSource(device);
    connect(probe, &QVideoProbe::videoFrameProbed,
            this, &Camera::processFrame, Qt::QueuedConnection);
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
    resolutions.clear();
    const QList<QSize> supportedResolutions = imagecapture->supportedResolutions();
    for (const QSize &res : supportedResolutions) {
        resolutions.append(res);
    }
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

void Camera::restart(int id)
{
    if (cameraID == id) {
        return;
    }
    stop();
    start(id, width, height);
    return;
}

void Camera::resize(int w, int h)
{
    if (width == w && height == h) {
        return;
    }
    Configuration::instance().setCameraParam(cameraID, w, h);
    /* resolution */
    QCameraViewfinderSettings settings;
#ifdef Q_OS_ANDROID
    settings.setPixelFormat(QVideoFrame::Format_NV21);
#else
    settings.setPixelFormat(QVideoFrame::Format_YUYV);
#endif
    settings.setResolution(QSize(w, h));
    device->setViewfinderSettings(settings);
    return;
}

void Camera::processFrame(const QVideoFrame &frame)
{
    process(frame);
    return;
}

void Camera::stop()
{
    if (imagecapture != nullptr) {
        imagecapture->deleteLater();
        imagecapture = nullptr;
    }
    if (probe != nullptr) {
        disconnect(probe, &QVideoProbe::videoFrameProbed,
                this, &Camera::processFrame);
        probe->flush();
        probe->deleteLater();
        probe = nullptr;
    }
    if (device != nullptr) {
        device->stop();
        device->deleteLater();
        device = nullptr;
    }
    return;
}

