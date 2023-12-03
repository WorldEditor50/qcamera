#ifndef PIPELINE_H
#define PIPELINE_H
#include <map>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <QVideoFrame>
#include "process_def.h"
#include "recorder.h"
#include "rtmppublisher.h"
#include "transmitter.h"
#include "improcess/transcoder.h"
#include "improcess/improcess.h"

#define USE_OPENGL 1

class Pipeline
{
public:
    enum State {
        STATE_NONE = 0,
        STATE_RUN,
        STATE_EMPTY,
        STATE_TERMINATE
    };
    using FnProcess = std::function<int(const cv::Mat &img, cv::Mat &out)>;
    constexpr static int max_thread_num = 1;
    constexpr static int max_queue_len = 8;
protected:
    State state;
    std::map<int, FnProcess> mapper;
    std::atomic<int> funcIndex;
    std::queue<QVideoFrame> frameQueue;
    cv::Mat out;
    std::mutex mutex;
    std::condition_variable condit;
    std::thread threads[max_thread_num];
protected:
    void run();
public:
    Pipeline();
    ~Pipeline();
    inline static Pipeline& instance()
    {
        static Pipeline pipeline;
        return pipeline;
    }
    void registerFunc(int index, const FnProcess &func);
    void dispatch(const QVideoFrame &frame);
    void setFunc(int index);
    void start();
    void stop();
};
#endif // PIPELINE_H
