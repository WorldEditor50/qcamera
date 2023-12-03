#ifndef YOLOV7_H
#define YOLOV7_H
// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "layer.h"
#include "net.h"

#if defined(USE_NCNN_SIMPLEOCV)
#include "simpleocv.h"
#else
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif
#include <float.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "yolo.h"

// original pretrained model from https://github.com/WongKinYiu/yolov7
// the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models

class Yolov7 : public Yolo
{
private:
    constexpr static int target_size = 160;
    constexpr static float prob_threshold = 0.25f;
    constexpr static float nms_threshold = 0.45f;
    constexpr static int MAX_STRIDE = 32;
public:
    Yolov7();
    ~Yolov7();
    inline static Yolov7& instance()
    {
        static Yolov7 yolo7;
        return yolo7;
    }
    int detect(const cv::Mat& bgr, std::vector<Object>& objects) override;
};

#endif // YOLOV7_H
