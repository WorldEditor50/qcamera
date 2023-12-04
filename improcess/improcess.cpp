#include "improcess.h"

int Improcess::color(const cv::Mat &img, cv::Mat &out)
{
    out = cv::Mat(img);
    return 0;
}

int Improcess::canny(const cv::Mat &img, cv::Mat &out)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);
    cv::blur(gray, gray, cv::Size(3, 3));
    cv::Canny(gray, out, 60, 120);
    return 0;
}

int Improcess::sobel(const cv::Mat &img, cv::Mat &out)
{
    /* gray */
    cv::Mat gray;
    cv::GaussianBlur(img, gray, cv::Size(3, 3), 0);
    cv::cvtColor(gray, gray, cv::COLOR_RGB2GRAY);
    /* x-grad */
    cv::Mat xGrad;
    cv::Sobel(gray, xGrad, CV_16S, 1, 0);
    cv::convertScaleAbs(xGrad, xGrad);
    /* y-grad */
    cv::Mat yGrad;
    cv::Sobel(gray, yGrad, CV_16S, 0, 1);
    cv::convertScaleAbs(yGrad, yGrad);
    /* merge gradient */
    cv::Mat grad;
    cv::addWeighted(xGrad, 0.5, yGrad, 0.5, 0, out);
    return 0;
}

int Improcess::laplace(const cv::Mat &img, cv::Mat &out)
{
    /* laplace  */
    cv::Mat gray;
    cv::GaussianBlur(img, gray, cv::Size(3, 3), 0);
    cv::cvtColor(gray, gray, cv::COLOR_RGB2GRAY);
    cv::Mat filterImg;
    cv::Laplacian(gray, filterImg, CV_16S, 3);
    cv::convertScaleAbs(filterImg, out);
    return 0;
}

int Improcess::haarcascade(const cv::Mat &img, cv::Mat &out)
{
    out = cv::Mat(img);
    std::vector<cv::Rect> boundingRects;
    Cascade::instance().detectFace(img, boundingRects);
    for (std::size_t i = 0; i < boundingRects.size(); i++) {
        cv::Rect &rect = boundingRects[i];
        cv::rectangle(out, rect, cv::Scalar(0, 255, 0), 2);
    }
    return 0;
}

int Improcess::yolov4(const cv::Mat &img, cv::Mat &out)
{
    std::vector<Yolov4::Object> objects;
    Yolov4::instance().detect(img, objects);
    out = cv::Mat(img);
    Yolov4::instance().draw(out, objects);
    return 0;
}

int Improcess::yolov5(const cv::Mat &img, cv::Mat &out)
{
    std::vector<Object> objects;
    Yolov5::instance().detect(img, objects);
    out = cv::Mat(img);
    Yolov5::instance().draw(out, objects);
    return 0;
}

int Improcess::yolov7(const cv::Mat &img, cv::Mat &out)
{
    std::vector<Object> objects;
    Yolov7::instance().detect(img, objects);
    out = cv::Mat(img);
    Yolov7::instance().draw(out, objects);
    return 0;
}

int Improcess::opticalFlow(const cv::Mat &img, cv::Mat &out)
{
    out = cv::Mat(img);
    OpticalFlow::instance().impl(img, out);
    return 0;
}

int Improcess::measure(const cv::Mat &img, cv::Mat &out)
{
    out = cv::Mat(img);
    /* convert to gray image */
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(7, 7), 0);
    /* detect edge */
    cv::Mat edged;
    cv::Canny(gray, edged, 50, 100);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::erode(edged, edged, kernel);
    cv::dilate(edged, edged, kernel);
    /* find contours */
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edged, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<int> indexes(contours.size());
    std::vector<float> areas(contours.size());
    for (std::size_t i = 0; i < contours.size(); i++) {
        indexes[i] = i;
        areas[i] = cv::contourArea(contours[i]);
    }
    std::sort(indexes.begin(), indexes.end(), [&](int i1, int i2)->bool {
        return areas[i1] > areas[i2];
    });
    
    for (int i : indexes) {
        if (areas[i] < 200) {
            continue;
        }
        cv::RotatedRect box = cv::minAreaRect(contours[i]);
        std::vector<cv::Point> points;
        cv::boxPoints(box, points);

        cv::drawContours(out, contours, i, cv::Scalar(0, 255, 0), 2);

    }
    return 0;
}

