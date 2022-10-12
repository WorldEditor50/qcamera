#ifndef PUBLISHER_H
#define PUBLISHER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
//#include "libavdevice/avdevice.h"
#include "libavutil/time.h"
#include <libavutil/opt.h>
}
#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class Publisher
{
public:
    AVFormatContext *outputContext;
    AVCodecContext *context;
    AVPacket *packet;
    int64_t framePts;

protected:
    /* frame */
    std::queue<AVFrame*> frameQueue;
    std::mutex mutex;
    std::condition_variable condit;
public:
    Publisher():outputContext(nullptr),context(nullptr),
            packet(nullptr),framePts(0){}
    ~Publisher();
    bool start(int width, int height,
              const char* formatName, const char* fileName);
    void operator()(AVFrame *frame);
    void stop();
    void impl();
};

#endif // PUBLISHER_H
