#ifndef YOLOV4_H
#define YOLOV4_H
#include "net.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <QVector>
#define YOLOV4_TINY //Using yolov4_tiny, if undef, using original yolov4

// original pretrained model from https://github.com/AlexeyAB/darknet
// the ncnn model https://drive.google.com/drive/folders/1YzILvh0SKQPS_lrb33dmGNq7aVTKPWS0?usp=sharing
// the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models

class Yolov4
{
public:
    struct Object {
        cv::Rect_<float> rect;
        int label;
        float prob;
    };
private:
    ncnn::Mutex lock;
private:

#ifdef YOLOV4_TINY
    constexpr static int target_size = 416;//416;
#else
    constexpr static int target_size = 608;
#endif
    bool hasLoadModel;
    ncnn::Net yolov4;
    ncnn::UnlockedPoolAllocator blob_pool_allocator;
    ncnn::PoolAllocator workspace_pool_allocator;
public:
    Yolov4();
    ~Yolov4();
    static Yolov4& instance()
    {
        static Yolov4 yolov4;
        return yolov4;
    }
    bool load(const std::string &modelType);
    void detect(const cv::Mat& image, std::vector<Object>& objects);
    void draw(cv::Mat& image, const std::vector<Object>& objects);

};

#endif // YOLOV4_H
