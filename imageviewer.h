#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QTouchEvent>
#include <QMouseEvent>

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
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
private:
    Ui::ImageViewer *ui;
    int touchCount;
};

#endif // IMAGEVIEWER_H
