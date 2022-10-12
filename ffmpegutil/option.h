#ifndef OPTION_H
#define OPTION_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libavdevice/avdevice.h"
}
#include <string>

struct Option
{ 
    static void rtmp(AVDictionary *options)
    {
        av_dict_set(&options, "rtmp_transport", "tcp", 0);
        av_dict_set(&options, "bufsize", "2000000", 0);
        av_dict_set(&options, "stimeout", "2000000", 0);
        return;
    }

    static void camera(AVDictionary *options, const std::string &res)
    {
        av_dict_set(&options, "video_size", res.c_str(), 0);
        av_dict_set(&options, "max_delay", "100000", 0);
        av_dict_set(&options, "framerate", "30", 0);
        /*
           pixel_format: yuyv422,mjpeg,nv12
        */
        av_dict_set(&options, "pixel_format", "yuyv422", 0);
        return;
    }

    static void flv(AVDictionary *options)
    {
        av_dict_set(&options, "flvflags", "no_duration_filesize", 0);
        return;
    }

};

#endif // OPTION_H
