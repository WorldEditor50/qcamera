#include "imageviewer.h"
#include "ui_imageviewer.h"

ImageViewer::ImageViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    connect(ui->backBtn, &QPushButton::clicked, this, &ImageViewer::back);
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
