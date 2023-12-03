#ifndef QSSLOADER_H
#define QSSLOADER_H
#include <QFile>

class QssLoader
{
public:
    static QString get(const QString &path)
    {
        QString qss;
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            qss = file.readAll();
            file.close();
        }
        return qss;
    }
};

#endif // QSSLOADER_H
