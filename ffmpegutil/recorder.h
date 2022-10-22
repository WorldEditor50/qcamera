#ifndef RECORDER_H
#define RECORDER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include <string>
#include <iostream>
#include <mutex>
#include <thread>

class Recorder
{
public:
    enum State {
        STATE_NONE = 0,
        STATE_RECORDING,
        STATE_FINISHED
    };
private:
    AVCodecContext* codecContext;
    AVFormatContext* formatContext;
    AVStream* outStream;
    AVFrame* yuv420pFrame;
    SwsContext *swsContext;
    AVFrame* frame;
    AVPacket *packet;
    AVPixelFormat pixelFormat;
    int pts;
    int state;
    std::mutex mutex;
private:
    Recorder();
public:
    inline static Recorder& instance()
    {
        static Recorder recorder;
        return recorder;
    }
    ~Recorder();
    int start(int width, int height, AVPixelFormat pixelFormat_, const std::string &videoFormat, const std::string &videoName);
    int rawEncode(unsigned char* data);
    int encode(AVFrame* frame);
    void stop();
    void clear();
};

#endif // RECORDER_H
