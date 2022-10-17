#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>

namespace Ui {
class ImageViewer;
}

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();
signals:
    void back();
public slots:
    void display(const QPixmap &pixmap);
private:
    Ui::ImageViewer *ui;
};

#endif // IMAGEVIEWER_H
