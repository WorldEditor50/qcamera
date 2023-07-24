#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include <QImage>
#include <map>
#include <functional>
#include <cmath>
#include <string>
#include <iostream>
#include "yolov4.h"
#include "yolov5.h"
#include "yolov7.h"
#include "cascade.h"

class OpticalFlow
{
private:
    cv::Mat preImage;
    cv::Ptr<cv::DenseOpticalFlow> denseOpticalFlow;
    std::vector<cv::Point2f> point1;
    std::vector<cv::Point2f> point2;
public:
    OpticalFlow():denseOpticalFlow(nullptr)
    {
        denseOpticalFlow = cv::DISOpticalFlow::create(cv::DISOpticalFlow::PRESET_FAST);
    }

    void operator()(cv::Mat &img)
    {
        if (preImage.empty()) {
            preImage = img.clone();
            return;
        }
        cv::Mat grayImg1;
        cv::cvtColor(img, grayImg1, cv::COLOR_RGBA2GRAY);
        cv::Mat grayImg2;
        cv::cvtColor(preImage, grayImg2, cv::COLOR_RGBA2GRAY);
        preImage = img.clone();
        cv::Mat flow;
        denseOpticalFlow->calc(grayImg1, grayImg2, flow);
        for (int i = 0; i < img.rows; i+=16) {
            for (int j = 0; j < img.cols; j+=16) {
                cv::Point2f offset = flow.at<cv::Point2f>(i, j);
                cv::line(img,
                        cv::Point(j, i),
                        cv::Point(cvRound(j + offset.x), cvRound(i + offset.y)),
                        CV_RGB(0, 255, 0), 1, 8);
            }
        }
        return;
    }
    void pyrLK(cv::Mat &img)
    {
        if (preImage.empty()) {
            preImage = img.clone();
            return;
        }
        cv::Mat grayImg1;
        cv::cvtColor(img, grayImg1, cv::COLOR_RGB2GRAY);
        cv::Mat grayImg2;
        cv::cvtColor(preImage, grayImg2, cv::COLOR_RGB2GRAY);
        preImage = img.clone();

        double qualityLevel = 0.01;
        double minDistance = 10;
        /* features */
        point2 = point1;
        cv::goodFeaturesToTrack(grayImg1, point1, 256, qualityLevel, minDistance);
        /* PyrLK */
        cv::Mat flow;
        cv::TermCriteria criteria(cv::TermCriteria::COUNT|cv::TermCriteria::EPS, 20, 0.03);
        std::vector<uchar> status;
        std::vector<float> err;
        cv::calcOpticalFlowPyrLK(grayImg1, grayImg2, point1, point2, status, err, cv::Size(9, 9), 3, criteria);
        for (size_t i = 0; i < point1.size() && i < point2.size(); i++){
            if (status[i] == 0 || (abs(point1[i].x - point2[i].x) + abs(point1[i].y-point2[i].y) < 2)) {
                continue;
            }
            cv::line(img,
                    cv::Point(cvRound(point1[i].x), cvRound(point1[i].y)),
                    cv::Point(cvRound(point2[i].x), cvRound(point2[i].y)),
                    cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
        }
        return;
    }
    static OpticalFlow& instance()
    {
        static OpticalFlow opticalFlow;
        return opticalFlow;
    }
};

class Improcess
{
public:
    inline static Improcess& instance()
    {
        static Improcess process;
        return process;
    }
    inline static cv::Mat color(int width, int height, unsigned char* data)
    {
        return cv::Mat(height, width, CV_8UC4, data);
    }

    inline static QImage mat2QImage(const cv::Mat &src)
    {
        QImage img;
        int channel = src.channels();
        switch (channel) {
        case 3:
            img = QImage(src.data, src.cols, src.rows, QImage::Format_RGB888);
            break;
        case 4:
            img = QImage(src.data, src.cols, src.rows, QImage::Format_ARGB32);
            break;
        default:
            img = QImage(src.cols, src.rows, QImage::Format_Indexed8);
            uchar *data = src.data;
            for (int i = 0; i < src.rows ; i++){
                uchar* rowdata = img.scanLine(i);
                for (int j = 0; j < src.cols; j++) {
                    rowdata[j] = data[j];
                }
                data += src.cols;
            }
            break;
        }
        return img;
    }
    static cv::Mat canny(int width, int height, unsigned char* data);
    static cv::Mat sobel(int width, int height, unsigned char *data);
    static cv::Mat laplace(int width, int height, unsigned char* data);
    static cv::Mat haarcascade(int width, int height, unsigned char* data);
    static cv::Mat yolov4(int width, int height, unsigned char* data);
    static cv::Mat yolov5(int width, int height, unsigned char* data);
    static cv::Mat yolov7(int width, int height, unsigned char* data);
    static cv::Mat opticalFlow(int width, int height, unsigned char* data);

private:
    Improcess(){}
};

#endif // IMAGEPROCESS_H
