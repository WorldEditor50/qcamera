#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <QObject>
#include <QImage>

class Transmitter : public QObject
{
    Q_OBJECT
public:
    Transmitter();
signals:
    void sendImage(const QImage &img);
    void sendGlImage(int h, int w, unsigned char* data);
public:
    inline static Transmitter& instance()
    {
        static Transmitter transmitter;
        return transmitter;
    }
};

#endif // TRANSMITTER_H
