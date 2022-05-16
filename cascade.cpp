#include "cascade.h"
#include <QDebug>

bool Cascade::load(const std::string &path)
{
#ifdef Q_OS_ANDROID
    cv::String faceCascadeModel = cv::String(path) + "/haarcascade_frontalface_alt.xml";
    cv::String eyesCascadeModel = cv::String(path) + "/haarcascade_eye_tree_eyeglasses.xml";
    cv::String upperbodyCascadeModel = cv::String(path) + "/haarcascade_upperbody.xml";
#else
    cv::String faceCascadeModel = "/home/eigen/Downloads/opencv-4.5.5/data/haarcascades/haarcascade_frontalface_alt.xml";
    cv::String eyesCascadeModel = "/home/eigen/Downloads/opencv-4.5.5/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
    cv::String upperbodyCascadeModel = "/home/eigen/Downloads/opencv-4.5.5/data/haarcascades/haarcascade_upperbody.xml";
#endif
    if (faceCascade.load(faceCascadeModel) == false) {
        qDebug()<<"failed to load face cascade.";
        return false;
    }
    if (eyesCascade.load(eyesCascadeModel) == false) {
        qDebug()<<"failed to load eyes cascade.";
        return false;
    }
    if (upperbodyCascade.load(upperbodyCascadeModel) == false) {
        qDebug()<<"failed to load upperbody cascade.";
        return false;
    }
    hasLoadModel = true;
    return true;
}

void Cascade::detect(cv::Mat &frame)
{
    if (hasLoadModel == true) {
        return;
    }
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_RGBA2GRAY);
    cv::equalizeHist(gray, gray);
    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(gray, faces);
    for (std::size_t i = 0; i < faces.size(); i++) {
        /* face */
        cv::Point center(faces[i].x + faces[i].width/2,
                         faces[i].y + faces[i].height/2);
        cv::ellipse(frame, center, cv::Size(faces[i].width/2, faces[i].height/2),
                    0, 0, 360, cv::Scalar(255, 0, 255), 4);
#if 0
        cv::Mat faceROI = gray(faces[i]);
        /* eyes */
        std::vector<cv::Rect> eyes;
        eyesCascade.detectMultiScale(faceROI, eyes);
        for (std::size_t j = 0; j < eyes.size(); j++) {
            cv::Point eyesCenter(faces[i].x + eyes[j].x + eyes[j].width/2,
                                 faces[i].y + eyes[j].y + eyes[j].height/2);
            int radius = cvRound((eyes[i].width + eyes[i].height)*0.25);
            cv::circle(frame, eyesCenter, radius, cv::Scalar(255, 0, 0), 4);
        }
        /* body */
        std::vector<cv::Rect> upperBodies;
        upperbodyCascade.detectMultiScale(gray, upperBodies);
        for (std::size_t i = 0; i < upperBodies.size(); i++) {
            cv::Point topLeft(upperBodies[i].x, upperBodies[i].y);
            cv::Point bottomRight(upperBodies[i].x + upperBodies[i].width,
                                  upperBodies[i].y + upperBodies[i].height);
            cv::rectangle(frame, topLeft, bottomRight, cv::Scalar(0, 255, 0), 4);
        }
#endif
    }
    return;
}

Cascade::Cascade():hasLoadModel(false)
{

}
