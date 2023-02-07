#include "videoinput.h"

VideoInput::VideoInput():context(nullptr)
{

}

VideoInput::~VideoInput()
{
    if (context != nullptr) {
        avformat_close_input(&context);
        avformat_free_context(context);
        context = nullptr;
    }
}

bool VideoInput::open(const std::string &url, const std::string &res, bool streamFlag)
{
    if (url.empty() || res .empty()) {
        return false;
    }
#ifdef WIN32
    const AVInputFormat *inputFormat = av_find_input_format(VIDEO_DEVICE);
#else
    const AVInputFormat *inputFormat = av_find_input_format(VIDEO_DEVICE);
#endif
    if (inputFormat == nullptr) {
        std::cout<<"failed to find format."<<std::endl;
        return false;
    }
    /* init */
    AVDictionary *options = nullptr;
    if (streamFlag == true) {
        av_dict_set(&options, "rtmp_transport", "tcp", 0);
        av_dict_set(&options, "bufsize", "2000000", 0);
        av_dict_set(&options, "stimeout", "2000000", 0);
    }
    /* show */
#if 0
    av_dict_set(&options, "list_devices", "true", 0);
    av_dict_set(&options, "list_options", "true", 0);
#endif
    av_dict_set(&options, "video_size", res.c_str(), 0);
    av_dict_set(&options, "max_delay", "100000", 0);
    //av_dict_set(&options, "framerate", "5", 0);
    /* pixel_format: yuyv422,mjpeg,nv12 */
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    /* input */
    int ret = avformat_open_input(&context, url.c_str(), inputFormat, &options);
    if (ret != 0) {
        std::cout<<"failed to open format, code:"<<ret;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        av_dict_free(&options);
        return false;
    }
    ret = avformat_find_stream_info(context, &options);
    if (ret < 0) {
        std::cout<<"failed to find stream information."<<std::endl;
        av_dict_free(&options);
        return false;
    }
    av_dict_free(&options);
    av_dump_format(context, 0, url.c_str(), 0);
    return true;
}
