#ifndef RTSPPUBLISHER_H
#define RTSPPUBLISHER_H



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

class RtspPublisher
{
public:
    enum State {
        STATE_NONE = 0,
        STATE_STREAMING,
        STATE_FINISHED
    };
public:
    AVFormatContext *outputContext;
    AVCodecContext *codecContext;
    AVStream *outStream;
    AVFrame* yuv420pFrame;
    SwsContext *swsContext;
    AVFrame* frame;
    AVPacket *packet;
    int64_t frameCount;
    int64_t startTime;
    AVRational timeBase;
    float timeBaseFloat;
    int64_t fps;
    std::mutex mutex;
    int state;
    bool isInited;
public:
    RtspPublisher():outputContext(nullptr),codecContext(nullptr),outStream(nullptr),
        yuv420pFrame(nullptr),swsContext(nullptr),frame(nullptr),
        packet(nullptr),frameCount(0),state(STATE_NONE),isInited(false){}
    ~RtspPublisher();
    static inline RtspPublisher& instance()
    {
        static RtspPublisher rtsp;
        return rtsp;
    }
    static inline double r2d(AVRational r)
    {
        return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
    }
    void enableNetwork();
    void start(int width, int height, const std::string &url);
    void encode(unsigned char *data);
    void stop();
    void clear();
};
#endif // RTSPPUBLISHER_H
