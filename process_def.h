#ifndef PROCESS_DEF_H
#define PROCESS_DEF_H

enum ProcessType {
    PROCESS_NONE = 0,
    PROCESS_COLOR,
    PROCESS_CANNY,
    PROCESS_SOBEL,
    PROCESS_LAPLACE,
    PROCESS_CASCADE,
    PROCESS_OPTICALFLOW,
    PROCESS_YOLOV5,
    PROCESS_YOLOV7
};

#endif // PROCESS_DEF_H