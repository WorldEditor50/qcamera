#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setting),
    camera(nullptr)
{
    ui->setupUi(this);
    /* rtmp */
    ui->streamUrlEdit->setPlaceholderText("rtsp://127.0.0.1/live/test");
    /* video format */
    ui->videoFormatComboBox->addItems(QStringList{"mp4", "avi", "flv"});
    connect(ui->videoFormatComboBox, &QComboBox::currentTextChanged, this, [=](const QString &text){
        Configuration::instance().setVideoFormat(text);
    });
    /* capture style */
    ui->styleComboBox->addItems(QStringList{"none"});
    connect(ui->styleComboBox, &QComboBox::currentTextChanged, this, [=](const QString &text){
        Configuration::instance().setCaptureStyle(text);
    });
    /* back */
    connect(ui->backBtn, &QPushButton::clicked, this, &Setting::back);
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
    /* rtmp */
    ui->streamUrlEdit->setText("rtsp://192.168.100.46/live/test");
    return;
}

QString Setting::getStreamURL()
{
    return ui->streamUrlEdit->text();
}

void Setting::updateDevice(int index)
{
    camera->restart(index);
    return;
}
