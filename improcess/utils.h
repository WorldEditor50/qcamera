#ifndef UTILS_H
#define UTILS_H
#include <cmath>
#include <opencv2/core.hpp>

namespace utils {

inline static float euclid(const cv::Point &p1, const cv::Point &p2)
{
    return std::sqrt((p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y));
}

}
#endif // UTILS_H
