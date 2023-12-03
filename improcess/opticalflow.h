#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

class OpticalFlow
{
private:
    cv::Mat preImage;
    cv::Ptr<cv::DenseOpticalFlow> denseOpticalFlow;
    std::vector<cv::Point2f> point1;
    std::vector<cv::Point2f> point2;
public:
    OpticalFlow();
    void impl(const cv::Mat &img, cv::Mat &out);
    void pyrLK(cv::Mat &img);
    static OpticalFlow& instance()
    {
        static OpticalFlow opticalFlow;
        return opticalFlow;
    }
};

#endif // OPTICALFLOW_H
