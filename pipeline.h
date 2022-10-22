#ifndef PIPELINE_H
#define PIPELINE_H
#include <QObject>
#include <map>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <string>
#include <cstring>
#include <QVideoFrame>
#include <QFile>
#include <QDateTime>
#include <QAtomicInt>
#include "libyuv.h"
#include "libyuv/convert_argb.h"
#include "imageprocess.h"
#include "recorder.h"
#include "rtmppublisher.h"

#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define USE_OPENGL 0
class ImageCache
{
private:
    std::mutex mutex;
    std::queue<unsigned char*> cache;
    constexpr static int max_cache_len = 16;
public:
    constexpr static int img_size = IMG_WIDTH*IMG_HEIGHT*4;
public:
    ImageCache(){}
    ~ImageCache()
    {
        clear();
    }
    void reserve(std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++) {
            unsigned char* ptr = new unsigned char[img_size];
            cache.push(ptr);
        }
        return;
    }
    void clear()
    {
        std::lock_guard<std::mutex> guard(mutex);
        for (std::size_t i = 0; i < cache.size(); i++) {
            unsigned char* ptr = cache.front();
            delete [] ptr;
            cache.pop();
        }
        return;
    }
    unsigned char* get()
    {
        std::lock_guard<std::mutex> guard(mutex);
        unsigned char* ptr = nullptr;
        if (cache.empty()) {
            ptr = new unsigned char[img_size];
        } else {
            ptr = cache.front();
            cache.pop();
        }
        memset(ptr, 0, img_size);
        return ptr;
    }
    void put(unsigned char* ptr)
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (cache.size() > max_cache_len) {
            delete [] ptr;
        } else {
            cache.push(ptr);
        }
        return;
    }
};

class Pipeline : public QObject
{
    Q_OBJECT
public:
    enum State {
        STATE_NONE = 0,
        STATE_RUN,
        STATE_EMPTY,
        STATE_TERMINATE
    };
    using Func = std::function<cv::Mat(int, int, unsigned char*)>;
    constexpr static int max_thread_num = 4;
    constexpr static int w = IMG_WIDTH;
    constexpr static int h = IMG_HEIGHT;
private:
    std::map<std::string, Func> mapper;
    std::queue<QVideoFrame> frameQueue;
    std::mutex mutex;
    std::condition_variable condit;
    std::thread threads[max_thread_num];
    State state;
    std::string funcName;
    ImageCache imgCache;
    QMatrix matrix;
public:
    inline static Pipeline& instance()
    {
        static Pipeline pipeline;
        return pipeline;
    }
    void registerFunc(const std::string &funcName_, const Func &func);
    void dispatch(const QVideoFrame &frame);
    void setFuncName(const std::string &funcName_);
    void start();
    void stop();
signals:
    void sendImage(const QImage &img);
    void sendGlImage(int w, int h, unsigned char* data);
private:
    Pipeline();
    ~Pipeline();
    void impl();
    static int transcode(QVideoFrame &frame, unsigned char* rgba);
};
#endif // PIPELINE_H
