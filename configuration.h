#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QSize>

class Configuration
{
private:
    QString storagePath;
    QString appDataPath;
    QString modelPath;
    QString picturePath;
    QString videoPath;
    QString videoFormat;
    QString captureStyle;
    int cameraID;
    bool isRoateFrame;
    QSize cameraSize;
private:
    Configuration();
public:
    static Configuration& instance()
    {
        static Configuration config;
        return config;
    }
    void load();
    /* get */
    QString getStoragePath() const {return storagePath;}
    QString getAppDataPath() const {return appDataPath;}
    QString getModelPath() const {return modelPath;}
    QString getPicturePath() const {return picturePath;}
    QString getVideoPath() const {return videoPath;}
    QString getVideoFormat() const {return videoFormat;}
    QString getCaptureStyle() const {return captureStyle;}
    int getCameraID() const {return cameraID;}
    inline bool isRotate() const { return isRoateFrame; }
    QSize getCameraSize() const {return cameraSize;}
    /* set */
    void setVideoFormat(const QString &format) {videoFormat = format;}
    void setCaptureStyle(const QString &style) {captureStyle = style;}
    void setCameraParam(int id, int w, int h) {cameraID = id;cameraSize = QSize(w, h);}
    void rotate() { isRoateFrame = isRoateFrame ? false : true;}
};

#endif // CONFIGURATION_H
