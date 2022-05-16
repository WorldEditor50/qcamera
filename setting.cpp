#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setting),
    camera(nullptr)
{
    ui->setupUi(this);
    connect(ui->selectBtn, &QPushButton::clicked, this, [=](){
        QString path = QFileDialog::getOpenFileName(nullptr, "select path", "/sdcard/");
        ui->savePathEdit->setText(path);
    });
    /* path */
    ui->savePathEdit->setPlaceholderText("/sdcard");
    /* rtmp */
    ui->rtmpEdit->setPlaceholderText("rtmp://47.106.179.135");
}

Setting::~Setting()
{
    delete ui;
}

void Setting::setDevice(Camera *camera_)
{
    if (camera_ == nullptr) {
        return;
    }
    camera = camera_;
    /* device */
    for (QCameraInfo& x : Camera::infoList) {
        ui->deviceComboBox->addItem(x.deviceName());
    }
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Setting::updateDevice);
    /* resolutions */
    for (QSize &x : camera->resolutions) {
        ui->resComboBox->addItem(QString("%1x%2").arg(x.width()).arg(x.height()));
    }
    /* path */
    ui->savePathEdit->setText("/sdcard");
    /* rtmp */
    ui->rtmpEdit->setText("rtmp://47.106.179.135");
    return;
}

void Setting::updateDevice(int index)
{
    camera->restart(index);
    return;
}
