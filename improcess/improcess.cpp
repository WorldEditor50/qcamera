#include "improcess.h"
#include "utils.h"
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
    /* filter noise */
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
    /* threshold */
    cv::Mat mask;
    cv::threshold(gray, mask, 0, 255, cv::THRESH_OTSU|cv::THRESH_BINARY_INV);
    /* dilate */
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(7, 7));
    cv::dilate(mask, mask, kernel);
    kernel = cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(7, 7));
    cv::erode(mask, mask, kernel);
    /* find contours */
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    double s = img.rows*img.cols;
    for (std::size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area < 400 || std::abs(area - s) < 50) {
            continue;
        }
        cv::RotatedRect box = cv::minAreaRect(contours[i]);
        cv::Mat points;
        /* bottom left, top left, top right, bottom right */
        cv::boxPoints(box, points);
        std::vector<cv::Point2i> vertices = {
            cv::Point2i(points.at<float>(0, 0), points.at<float>(0, 1)),
            cv::Point2i(points.at<float>(1, 0), points.at<float>(1, 1)),
            cv::Point2i(points.at<float>(2, 0), points.at<float>(2, 1)),
            cv::Point2i(points.at<float>(3, 0), points.at<float>(3, 1))};
        /* messure */
        float d1 = utils::euclid(vertices[0], vertices[1]);
        float d2 = utils::euclid(vertices[1], vertices[2]);
        /* draw vertex */
        for (std::size_t j = 0; j < vertices.size(); j++) {
            cv::circle(out, vertices[j], 8, cv::Scalar(0, 0, 255), -1);
        }
        /* line vertex */
        cv::line(out, vertices[0], vertices[1], cv::Scalar(0, 255, 0), 2);
        cv::line(out, vertices[1], vertices[2], cv::Scalar(0, 255, 0), 2);
        cv::line(out, vertices[2], vertices[3], cv::Scalar(0, 255, 0), 2);
        cv::line(out, vertices[3], vertices[0], cv::Scalar(0, 255, 0), 2);
        /* draw midpoints */
        std::vector<cv::Point2i> midpoints(4);
        midpoints[0] = (vertices[0] + vertices[1])/2;
        midpoints[1] = (vertices[1] + vertices[2])/2;
        midpoints[2] = (vertices[2] + vertices[3])/2;
        midpoints[3] = (vertices[3] + vertices[0])/2;
        for (std::size_t j = 0; j < midpoints.size(); j++) {
            cv::circle(img, midpoints[j], 8, cv::Scalar(255, 0, 0), -1);
        }
        /* line midpoint */
        cv::line(out, midpoints[0], midpoints[2], cv::Scalar(255, 0, 255), 2);
        cv::line(out, midpoints[1], midpoints[3], cv::Scalar(255, 0, 255), 2);
        /* draw messure */
        cv::putText(out, std::to_string(d1), midpoints[0],
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
        cv::putText(out, std::to_string(d2), midpoints[1],
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
    }
    return 0;
}

