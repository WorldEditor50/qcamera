#include "pipeline.h"

void Pipeline::registerFunc(int index, const Pipeline::FnProcess &func)
{
    mapper.insert(std::pair<int, FnProcess>(index, func));
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

void Pipeline::setFunc(int index)
{
    funcIndex.store(index);
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

Pipeline::Pipeline():
    QObject(nullptr)
{
    /* method */
    mapper.insert(std::pair<int, FnProcess>(PROCESS_CANNY, &Improcess::canny));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_SOBEL, &Improcess::sobel));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_LAPLACE, &Improcess::laplace));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_OPTICALFLOW, &Improcess::opticalFlow));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_CASCADE, &Improcess::haarcascade));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_YOLOV5, &Improcess::yolov5));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_YOLOV7, &Improcess::yolov7));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_COLOR, &Improcess::color));
    matrix.rotate(90.0);
    /* function */
    funcIndex = PROCESS_COLOR;
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
            std::unique_lock<std::mutex> locker(mutex);
            condit.wait(locker, [&]()->bool{
                            return (frameQueue.empty() == false || state == STATE_TERMINATE);
                        });
            if (state == STATE_TERMINATE) {
                break;
            }
            frame = std::move(frameQueue.front());
            frameQueue.pop();
            if (frameQueue.size() > 32) {
                continue;
            }
        }
        /* process */
        int index = funcIndex.load();
        auto it = mapper.find(index);
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

        cv::Mat img = it->second(w, h, rgba);
        if (img.empty()) {
            imgCache.put(rgba);
            continue;
        }
        if (img.channels() == 1) {
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        } else if (img.channels() == 4) {
            cv::cvtColor(img, img, cv::COLOR_RGBA2BGR);
        }
#if USE_OPENGL
        emit sendGlImage(img.cols, img.rows, img.data);
#else
        QImage image = Improcess::mat2QImage(img);
#ifdef Q_OS_ANDROID
        image = image.transformed(matrix, Qt::FastTransformation);
#endif
        emit sendImage(image);
#endif
        /* record */
        Recorder::instance().rawEncode(img.data);

        /* stream */
        RtmpPublisher::instance().encode(img.data);
        imgCache.put(rgba);
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

