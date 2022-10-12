#ifndef DECODER_H
#define DECODER_H
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include <libavutil/opt.h>
}
#include <iostream>
#include <functional>
#include <string>

class Decoder
{
public:
    AVFormatContext *inputContext;
    AVCodecContext *context;
    AVFrame *frame;
    AVPacket *packet;
    int videoStreamIndex;
public:
    Decoder():inputContext(nullptr),context(nullptr),
        frame(nullptr),packet(nullptr){}
    ~Decoder();
    bool open(AVFormatContext *inputContext_);
    bool open();
    void close();
    void operator()(std::function<void(AVFrame*)> process);
};

#endif // DECODER_H
