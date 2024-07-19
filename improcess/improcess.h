#ifndef IMPROCESS_H
#define IMPROCESS_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <map>
#include <functional>
#include <cmath>
#include <string>
#include <iostream>
#include "yolov4.h"
#include "yolov5.h"
#include "yolov7.h"
#include "cascade.h"
#include "opticalflow.h"

class Improcess
{
public:
    static int color(const cv::Mat &img, cv::Mat &out);
    static int canny(const cv::Mat &img, cv::Mat &out);
    static int sobel(const cv::Mat &img, cv::Mat &out);
    static int laplace(const cv::Mat &img, cv::Mat &out);
    static int haarcascade(const cv::Mat &img, cv::Mat &out);
    static int yolov4(const cv::Mat &img, cv::Mat &out);
    static int yolov5(const cv::Mat &img, cv::Mat &out);
    static int yolov7(const cv::Mat &img, cv::Mat &out);
    static int opticalFlow(const cv::Mat &img, cv::Mat &out);
    static int measure(const cv::Mat &img, cv::Mat &out);
    static int cluster(const cv::Mat &img, cv::Mat &out);
};

#endif // IMAGEPROCESS_H
