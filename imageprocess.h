#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QImage>
#include <map>
#include <functional>
#include <cmath>
#include <string>
#include "yolov5.h"
#include "yolov4.h"
#include "cascade.h"

class Process
{
public:
    enum FuncType {
        FUNC_COLOR = 0,
        FUNC_CANNY,
        FUNC_SOBEL,
        FUNC_LAPLACE,
        FUNC_HAARCASCADE,
        FUNC_YOLOV4,
        FUNC_YOLOV5
    };
public:
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
    static QImage canny(int width, int height, unsigned char* data);
    static QImage sobel(int width, int height, unsigned char *data);
    static QImage laplace(int width, int height, unsigned char* data);
    static QImage haarcascade(int width, int height, unsigned char* data);
    static QImage yolov4Detect(int width, int height, unsigned char* data);
    static QImage yolov5Detect(int width, int height, unsigned char* data);
    inline static QImage color(int width, int height, unsigned char* data)
    {
        return QImage(data, width, height, QImage::Format_ARGB32);
    }
    inline static Process& instance()
    {
        static Process process;
        return process;
    }
private:
    Process(){}
};

#endif // IMAGEPROCESS_H
