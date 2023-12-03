#include "opticalflow.h"

OpticalFlow::OpticalFlow()
{
    denseOpticalFlow = cv::DISOpticalFlow::create(cv::DISOpticalFlow::PRESET_FAST);
}

void OpticalFlow::impl(const cv::Mat &img, cv::Mat &out)
{
    if (preImage.empty()) {
        preImage = cv::Mat(img);
        return;
    }
    cv::Mat grayImg1;
    cv::cvtColor(img, grayImg1, cv::COLOR_RGB2GRAY);
    cv::Mat grayImg2;
    cv::cvtColor(preImage, grayImg2, cv::COLOR_RGB2GRAY);
    preImage = cv::Mat(img);
    cv::Mat flow;
    denseOpticalFlow->calc(grayImg1, grayImg2, flow);
    for (int i = 0; i < img.rows; i+=16) {
        for (int j = 0; j < img.cols; j+=16) {
            cv::Point2f offset = flow.at<cv::Point2f>(i, j);
            cv::line(out,
                     cv::Point(j, i),
                     cv::Point(cvRound(j + offset.x), cvRound(i + offset.y)),
                     cv::Scalar(0, 255, 0), 1, 8);
        }
    }
    return;
}
void OpticalFlow::pyrLK(cv::Mat &img)
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
