#include "imageviewer.h"
#include "ui_imageviewer.h"

ImageViewer::ImageViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageViewer),touchCount(0)
{
    ui->setupUi(this);
    ui->imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::display(const QPixmap &pixmap)
{
    QWidget* window = ui->imageWidget;
    int w = window->width();
    int h = window->height();
    if (pixmap.width() >= w && pixmap.height() >= h) {
        ui->imageLabel->resize(w, h);
    } else if (pixmap.width() >= w && pixmap.height() < h) {
        ui->imageLabel->resize(w, pixmap.height());
    } else if (pixmap.width() < w && pixmap.height() >= h) {
        ui->imageLabel->resize(pixmap.width(), h);
    } else if (pixmap.width() < w && pixmap.height() < h) {
        ui->imageLabel->resize(pixmap.width(), pixmap.height());
    }
    ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size()));
    return;
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit back();
    }
    return QWidget::mouseDoubleClickEvent(event);
}

bool ImageViewer::event(QEvent *ev)
{
    switch (ev->type()) {
    case QTouchEvent::TouchBegin:
    case QTouchEvent::TouchUpdate:
        ev->accept();
        touchCount++;
        update();
        break;
    case QTouchEvent::TouchEnd:
        if (touchCount == 2) {
            emit back();
            touchCount = 0;
        }
        update();
        break;
    default:
        break;
    }
    return QWidget::event(ev);
}
