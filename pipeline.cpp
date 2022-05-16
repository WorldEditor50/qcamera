#include "pipeline.h"

void Pipeline::registerFunc(const std::string &funcName_, const Pipeline::Func &func)
{
    mapper.insert(std::pair<std::string, Func>(funcName_, func));
    return;
}

void Pipeline::dispatch(const QVideoFrame &frame)
{
    {
         std::unique_lock<std::mutex> guard(mutex);
         if (state == STATE_TERMINATE || state == STATE_NONE) {
             return;
         }
    }
    std::unique_lock<std::mutex> guard(mutex);
    frameQueue.push(frame);
    condit.notify_one();
    return;
}

void Pipeline::setFuncName(const std::string &funcName_)
{
    funcName = funcName_;
    return;
}

void Pipeline::start()
{
    state = STATE_RUN;
    for (int i = 0; i < max_thread_num; i++) {
        threads[i] = std::thread(&Pipeline::impl, this);
    }
    return;
}

void Pipeline::stop()
{
    if (state != STATE_TERMINATE) {
        std::unique_lock<std::mutex> guard(mutex);
        state = STATE_TERMINATE;
        condit.notify_all();
    }
    for (int i = 0; i < max_thread_num; i++) {
        threads[i].join();
    }
    return;
}

Pipeline::Pipeline():QObject(nullptr)
{
    /* method */
    mapper.insert(std::pair<std::string, Func>("canny", &Process::canny));
    mapper.insert(std::pair<std::string, Func>("sobel", &Process::sobel));
    mapper.insert(std::pair<std::string, Func>("laplace", &Process::laplace));
    mapper.insert(std::pair<std::string, Func>("haarcascade", &Process::haarcascade));
    mapper.insert(std::pair<std::string, Func>("yolov4", &Process::yolov4Detect));
    mapper.insert(std::pair<std::string, Func>("yolov5", &Process::yolov5Detect));
    mapper.insert(std::pair<std::string, Func>("color", &Process::color));
    matrix.rotate(90.0);
    /* function */
    funcName = "color";
    imgCache.reserve(8);
}

Pipeline::~Pipeline()
{
    stop();
}

void Pipeline::impl()
{
    while (1) {
        QVideoFrame frame;
        {
            std::unique_lock<std::mutex> guard(mutex);
            condit.wait(guard, [this](){return (frameQueue.empty() == false || state == STATE_TERMINATE);});
            if (state == STATE_TERMINATE) {
                break;
            }
            frame = std::move(frameQueue.front());
            frameQueue.pop();
            if (frameQueue.size() > max_video_queue_len) {
                continue;
            }
        }
        /* process */
        auto it = mapper.find(funcName);
        if (it == mapper.end()) {
            continue;
        }
        unsigned char* rgba = imgCache.get();
        if (rgba == nullptr) {
            continue;
        }
        /* transcode */
        int ret = transcode(frame, rgba);
        if (ret != 0) {
            imgCache.put(rgba);
            continue;
        }

        QImage img = it->second(w, h, rgba);
        if (img.isNull()) {
            imgCache.put(rgba);
            continue;
        }
#ifdef Q_OS_ANDROID
        img = img.transformed(matrix, Qt::FastTransformation);
#endif
        imgCache.put(rgba);
        emit sendImage(img);
    }
    return;
}

int Pipeline::transcode(QVideoFrame &frame, unsigned char *rgba)
{
    frame.map(QAbstractVideoBuffer::ReadOnly);
    int w = frame.width();
    int h = frame.height();
    int ret = -1;
    if (frame.pixelFormat() == QVideoFrame::Format_NV21) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV21ToARGB(y, w, uv, w, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_NV12) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV12ToARGB(y, w, uv, w, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUYV) {
        int alignedWidth = (w + 1) & ~1;
        ret = libyuv::YUY2ToARGB(frame.bits(), alignedWidth*2, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUV420P) {
        unsigned char *y = frame.bits();
        unsigned char *u = frame.bits() + h*w;
        unsigned char *v = frame.bits() + h*w*5/4;
        ret = libyuv::I420ToARGB(y, w, u, w/2, v, w/2, rgba, w*4, w, h);
    }
    frame.unmap();
    return ret;
}

