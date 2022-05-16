#ifndef CASCADE_H
#define CASCADE_H

#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

class Cascade
{
private:
    bool hasLoadModel;
    cv::CascadeClassifier faceCascade;
    cv::CascadeClassifier eyesCascade;
    cv::CascadeClassifier upperbodyCascade;
public:
    inline static Cascade& instance()
    {
        static Cascade cascade;
        return cascade;
    }
    bool load(const std::string &path);
    void detect(cv::Mat &frame);
private:
    Cascade();
};

#endif // CASCADE_H
