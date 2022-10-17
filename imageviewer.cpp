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
    ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size()));
    return;
}
