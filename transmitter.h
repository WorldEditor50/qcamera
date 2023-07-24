#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <QObject>

class Transmitter : public QObject
{
    Q_OBJECT
public:
    explicit Transmitter(QObject *parent = nullptr);

signals:

};

#endif // TRANSMITTER_H
