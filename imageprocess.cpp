#include "imageprocess.h"
#include <QDebug>

QImage Process::canny(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    cv::cvtColor(src, src, cv::COLOR_RGBA2GRAY);
    cv::blur(src, src, cv::Size(3, 3));
    cv::Canny(src, src, 60, 120);
    return mat2QImage(src);
}

QImage Process::sobel(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    /* gray */
    cv::Mat gray;
    cv::GaussianBlur(src, gray, cv::Size(3, 3), 0);
    cv::cvtColor(gray, gray, cv::COLOR_RGBA2GRAY);
    /* x-grad */
    cv::Mat xGrad;
    cv::Sobel(gray, xGrad, CV_16S, 1, 0);
    cv::convertScaleAbs(xGrad, xGrad);
    /* y-grad */
    cv::Mat yGrad;
    cv::Sobel(gray, yGrad, CV_16S, 0, 1);
    cv::convertScaleAbs(yGrad, yGrad);
    /* merge gradient */
    cv::addWeighted(xGrad, 0.5, yGrad, 0.5, 0, src);
    return mat2QImage(src);
}

QImage Process::laplace(int width, int height, unsigned char *data)
{
    /* laplace  */
    cv::Mat src(height, width, CV_8UC4, data);
    cv::Mat gray;
    cv::GaussianBlur(src, gray, cv::Size(3, 3), 0);
    cv::cvtColor(gray, gray, cv::COLOR_RGBA2GRAY);
    cv::Mat filterImg;
    cv::Laplacian(gray, filterImg, CV_16S, 3);
    cv::Mat dst;
    cv::convertScaleAbs(filterImg, dst);
    return mat2QImage(dst);
}

QImage Process::haarcascade(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    Cascade::instance().detect(src);
    return mat2QImage(src);
}

QImage Process::yolov4Detect(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    {
        ncnn::MutexLockGuard guard(Yolov4::instance().lock);
        QVector<Yolov4::Object> objects;
        Yolov4::instance().detect(src, objects);
        Yolov4::instance().draw(src, objects);
    }
    return mat2QImage(src);
}

QImage Process::yolov5Detect(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    {
        ncnn::MutexLockGuard guard(Yolov5::instance().lock);
        QVector<Yolov5::Object> objects;
        Yolov5::instance().detect(src, objects);
        Yolov5::instance().draw(src, objects);
    }
    return mat2QImage(src);
}

