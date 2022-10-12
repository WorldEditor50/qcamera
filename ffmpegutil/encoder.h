#ifndef ENCODER_H
#define ENCODER_H
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
#include <map>
class Encoder
{
public:
    AVFormatContext *outputContext;
    AVCodecContext *context;
    AVPacket *packet;
    int64_t framePts;
public:
    Encoder():outputContext(nullptr),context(nullptr),
            packet(nullptr),framePts(0){}
    ~Encoder();
    bool open(int width, int height,
              const std::string &formatName, const std::string &fileName);
    void operator()(AVFrame *frame);
    void flush();
};

#endif // ENCODER_H
