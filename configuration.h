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
public:
    static Configuration& instance()
    {
        static Configuration config;
        return config;
    }
    void load();
    QString getStoragePath() const {return storagePath;}
    QString getAppDataPath() const {return appDataPath;}
    QString getModelPath() const {return modelPath;}
    QString getPicturePath() const {return picturePath;}
    QString getVideoPath() const {return videoPath;}
private:
    Configuration();
};

#endif // CONFIGURATION_H
