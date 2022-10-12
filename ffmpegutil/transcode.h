#ifndef TRANSCODE_H
#define TRANSCODE_H
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
}
#include <array>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>

template <int width, int height, AVPixelFormat format, bool allocateData>
class FrameAllocator
{
public:
    std::queue<AVFrame*> frames;
    std::mutex mutex;
public:
    AVFrame* get()
    {
        std::lock_guard<std::mutex> guard(mutex);
        AVFrame* frame = nullptr;
        if (frames.empty()) {
            frame = av_frame_alloc();
            frame->format = format;
            frame->width  = width;
            frame->height = height;
            if (allocateData == true) {
                av_frame_get_buffer(frame, 0);
            }
        } else {
            frame = frames.front();
            frames.pop();
        }
        return frame;
    }

    void push(AVFrame* frame)
    {
        std::lock_guard<std::mutex> guard(mutex);
        frames.push(frame);
        return;
    }
};



template <int width, int height, AVPixelFormat srcFormat, AVPixelFormat dstFormat>
class Transcode
{
public:
    constexpr static int max_frame_num = 8;
private:
    std::array<AVFrame*, max_frame_num> srcFrames;
    std::array<AVFrame*, max_frame_num> dstFrames;
    std::size_t index;
    std::mutex mutex;
    SwsContext *context;
public:
    Transcode():index(0),context(nullptr)
    {
        std::lock_guard<std::mutex> guard(mutex);
        /* swscontex */
        context = sws_getContext(width, height, srcFormat,
                                 width, height, dstFormat,
                                 SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        /* src frame */
        for (std::size_t i = 0; i < srcFrames.size(); i++) {
            srcFrames[i] = av_frame_alloc();
            srcFrames[i]->format = srcFormat;
            srcFrames[i]->width  = width;
            srcFrames[i]->height = height;
            //av_frame_get_buffer(srcFrames[i], 0);
        }
        /* dst frame */
        for (std::size_t i = 0; i < dstFrames.size(); i++) {
            dstFrames[i] = av_frame_alloc();
            dstFrames[i]->format = srcFormat;
            dstFrames[i]->width  = width;
            dstFrames[i]->height = height;
            av_frame_get_buffer(dstFrames[i], 0);
        }
    }

    ~Transcode()
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (context != nullptr) {
            sws_freeContext(context);
            context = nullptr;
        }
        for (AVFrame* frame : srcFrames) {
            av_frame_unref(frame);
            av_frame_free(&frame);
            frame = nullptr;
        }
        for (AVFrame* frame : dstFrames) {
            av_frame_unref(frame);
            av_frame_free(&frame);
            frame = nullptr;
        }
    }
    AVFrame* operator()(unsigned char* data, int len)
    {
        /* get frame */
        AVFrame* src = nullptr;
        AVFrame* dst = nullptr;
        {
            std::lock_guard<std::mutex> guard(mutex);
            src = srcFrames[index];
            dst = dstFrames[index];
            index = (index + 1)%max_frame_num;
        }
        if (src == nullptr || dst == nullptr) {
            return nullptr;
        }
        /* assign data */
//        int ret = av_frame_make_writable(dst);
//        if (ret < 0) {
//            return nullptr;
//        }
        //memcpy(src->data[0], data, len);
        int ret = av_image_fill_arrays(src->data, src->linesize,
                                       data, srcFormat, width, height, 2);
        if (ret != len) {
            printf("invalid frame, ret =%d\n", ret);
            return nullptr;
        }
        /* transcode */
        sws_scale(context, src->data, src->linesize, 0,
                  height, dst->data, dst->linesize);

        return dst;
    }
};

#endif // TRANSCODE_H
