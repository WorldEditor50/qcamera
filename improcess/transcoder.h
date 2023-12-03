#ifndef TRANSCODER_H
#define TRANSCODER_H
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <libyuv.h>
#include <libyuv/convert_argb.h>
#include <QImage>
#include <QVideoFrame>


class Transcoder
{
public:
    Transcoder();
    static QImage fromMat(const cv::Mat &src);
    static int videoFrameToRGBA(QVideoFrame &frame, int &h, int &w, unsigned char *rgba);
    static int videoFrameToMat(QVideoFrame &frame, cv::Mat &img);
    static void nv21ToRGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height);
    static void yuv420pToRGB24(unsigned char *yuv420p, unsigned char *rgb24, int width, int height);
    static void yuyvToRGB24(unsigned char *yuv_buffer,unsigned char *rgb_buffer,int iWidth,int iHeight);
};

#endif // TRANSCODER_H
