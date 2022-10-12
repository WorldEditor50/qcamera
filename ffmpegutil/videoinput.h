#ifndef VIDEOINPUT_H
#define VIDEOINPUT_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
//#include "libavdevice/avdevice.h"
}

#include <string>
#include <iostream>

#ifdef _WIN32
#define VIDEO_DEVICE "dshow"
#endif

#ifdef __linux__
#define VIDEO_DEVICE "video4linux2"
#endif

#ifdef __APPLE__
#define VIDEO_DEVICE "avfoundation"
#endif

class VideoInput
{
public:
    AVFormatContext *context;
public:
    VideoInput();
    ~VideoInput();
    bool open(const std::string &url, const std::string &res, bool streamFlag);
};

#endif // VIDEOINPUT_H
