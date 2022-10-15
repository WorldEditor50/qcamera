#ifndef YOLOV5_H
#define YOLOV5_H
#include "layer.h"
#include "net.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <float.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "yolo.h"

#define YOLOV5_V60 1 //YOLOv5 v6.0

// original pretrained model from https://github.com/ultralytics/yolov5
// the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models
class Yolov5 : public Yolo
{
protected:
    constexpr static int target_size = 160;
    constexpr static float prob_threshold = 0.25f;
    constexpr static float nms_threshold = 0.45f;
    constexpr static int MAX_STRIDE = 64;
    ncnn::Mutex lock;
    ncnn::UnlockedPoolAllocator blob_pool_allocator;
    ncnn::PoolAllocator workspace_pool_allocator;
private:
    Yolov5();
public:
    static Yolov5& instance()
    {
        static Yolov5 yolov5;
        return yolov5;
    }
    int detect(const cv::Mat &image, std::vector<Object> &objects) override;

};

#endif // YOLOV5_H
