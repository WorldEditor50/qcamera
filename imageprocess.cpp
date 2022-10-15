#include "imageprocess.h"
#include <QDebug>

cv::Mat Improcess::canny(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    cv::cvtColor(src, src, cv::COLOR_RGBA2GRAY);
    cv::blur(src, src, cv::Size(3, 3));
    cv::Canny(src, src, 60, 120);
    return src;
}

cv::Mat Improcess::sobel(int width, int height, unsigned char *data)
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
    return src;
}

cv::Mat Improcess::laplace(int width, int height, unsigned char *data)
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
    return dst;
}

cv::Mat Improcess::haarcascade(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    Cascade::instance().detect(src);
    return src;
}

cv::Mat Improcess::yolov4(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    {
        ncnn::MutexLockGuard guard(Yolov4::instance().lock);
        QVector<Yolov4::Object> objects;
        Yolov4::instance().detect(src, objects);
        Yolov4::instance().draw(src, objects);
    }
    return src;
}

cv::Mat Improcess::yolov5(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    std::vector<Object> objects;
    Yolov5::instance().detect(src, objects);
    Yolov5::instance().draw(src, objects);
    return src;
}

cv::Mat Improcess::yolov7(int width, int height, unsigned char *data)
{
    cv::Mat src(height, width, CV_8UC4, data);
    std::vector<Object> objects;
    Yolov7::instance().detect(src, objects);
    Yolov7::instance().draw(src, objects);
    return src;
}

