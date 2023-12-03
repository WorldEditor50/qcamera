#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDir>
#include <QStandardPaths>
#include <QString>

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
    /* set */
    void setVideoFormat(const QString format);
    void setCaptureStyle(const QString style);
    void setCameraID(int id) {cameraID = id;}
private:
    Configuration();
};

#endif // CONFIGURATION_H
