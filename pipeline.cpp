#include "pipeline.h"

void Pipeline::registerFunc(int index, const Pipeline::FnProcess &func)
{
    mapper.insert(std::pair<int, FnProcess>(index, func));
    return;
}

void Pipeline::dispatch(const QVideoFrame &frame)
{
    if (state == STATE_TERMINATE || state == STATE_NONE) {
        return;
    }
    std::unique_lock<std::mutex> guard(mutex);
    frameQueue.push(frame);
    if (frameQueue.size() > max_queue_len) {
        frameQueue.pop();
    }
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
        threads[i] = std::thread(&Pipeline::run, this);
    }
    return;
}

void Pipeline::stop()
{
    if (state == STATE_NONE) {
        return;
    }
    {
        std::unique_lock<std::mutex> guard(mutex);
        state = STATE_TERMINATE;
        condit.notify_all();
    }
    for (int i = 0; i < max_thread_num; i++) {
        threads[i].join();
    }
    return;
}

Pipeline::Pipeline()
{
    /* method */
    mapper.insert(std::pair<int, FnProcess>(PROCESS_CANNY, &Improcess::canny));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_SOBEL, &Improcess::sobel));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_LAPLACE, &Improcess::laplace));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_OPTICALFLOW, &Improcess::opticalFlow));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_MEASSURE, &Improcess::measure));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_CASCADE, &Improcess::haarcascade));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_YOLOV5, &Improcess::yolov5));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_YOLOV7, &Improcess::yolov7));
    mapper.insert(std::pair<int, FnProcess>(PROCESS_COLOR, &Improcess::color));
    /* function */
    funcIndex = PROCESS_COLOR;
}

Pipeline::~Pipeline()
{
    stop();
}

void Pipeline::run()
{
    while (1) {
        QVideoFrame frame;
        {
            std::unique_lock<std::mutex> locker(mutex);
            condit.wait(locker, [&]()->bool{
                            return (frameQueue.empty() == false || state == STATE_TERMINATE);
                        });
            if (state == STATE_TERMINATE) {
                state = STATE_NONE;
                break;
            }
            frame = std::move(frameQueue.front());
            frameQueue.pop();
            if (frameQueue.size() > max_queue_len) {
                continue;
            }
        }
        /* transcode */
        cv::Mat img;
        int ret = Transcoder::videoFrameToMat(frame, img, Configuration::instance().isRotate());
        if (ret != 0) {
            continue;
        }
#ifdef Q_OS_ANDROID
        if (Configuration::instance().getCameraID() == 1) {
            cv::flip(img, img, 0);
        }
#endif
        /* process */
        int index = funcIndex.load();
        auto it = mapper.find(index);
        if (it == mapper.end()) {
            continue;
        }
        it->second(img, out);
        if (out.empty()) {
            continue;
        }
#if USE_OPENGL
        if (out.channels() == 1) {
            cv::cvtColor(out, out, cv::COLOR_GRAY2RGB);
        }
        Transmitter::instance().sendGlImage(out.rows, out.cols, out.data);
#else
        QImage image = Transcoder::fromMat(out);
        Transmitter::instance().sendImage(image);
#endif
        /* record */
        Recorder::instance().rawEncode(out.data);
        /* stream */
#if 0
        RtspPublisher::instance().encode(out.data);
#endif
    }
    std::cout<<"pipeline leave process"<<std::endl;
    return;
}

