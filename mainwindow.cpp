#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pageID(0)
    , readyQuit(false)
    , readyCapture(0)
{
    ui->setupUi(this);
    /* opengl */
#if USE_OPENGL
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setProfile(QSurfaceFormat::CoreProfile);
    ui->openGLWidget->setFormat(format);
    ui->stackedWidget->setCurrentIndex(PAGE_OPENGL_VIDEO);
#else
    ui->stackedWidget->setCurrentIndex(PAGE_VIDEO);
    ui->videoLabel->show();
#endif

#ifdef Q_OS_ANDROID
    setAttribute(Qt::WA_AcceptTouchEvents);
#else
    setMinimumSize(QSize(800, 600));
#endif
    /* menu */
    createMenu();
    /* load model */
    QtConcurrent::run([=](){
        bool ret = true;
#ifdef Q_OS_ANDROID
        QString modelPath = Configuration::instance().getModelPath();
        ret = Yolov7::instance().load(QString("%1/yolov7-tiny").arg(modelPath).toStdString());
        ret = Yolov5::instance().load(QString("%1/yolov5s_6.0").arg(modelPath).toStdString());
        ret = Yolov4::instance().load(QString("%1/yolov4-tiny-opt").arg(modelPath).toStdString());
        ret = Cascade::instance().load(modelPath.toStdString());
#else
        ret = Yolov7::instance().load("/home/eigen/MySpace/models/ncnn-assets/models/yolov7-tiny");
        ret = Yolov5::instance().load("/home/eigen/MySpace/models/ncnn-assets/models/yolov5s_6.0");
        ret = Yolov4::instance().load("/home/eigen/MySpace/models/ncnn-assets/models/yolov4-tiny-opt");
        ret = Cascade::instance().load("");
#endif
        if (ret == false) {
            statusBar()->showMessage("Failed to load model.");
        } else {
            statusBar()->showMessage("load model completed.");
        }
    });

    /* camera */
    connect(this, &MainWindow::send, this, &MainWindow::updateImage, Qt::QueuedConnection);
    Camera::infoList = QCameraInfo::availableCameras();
    /* start camera */
    camera = new Camera(this);
    camera->start(0);
    ui->settingWidget->setDevice(camera);
#if USE_OPENGL
    connect(&Pipeline::instance(), &Pipeline::sendGlImage,
            this, &MainWindow::updateGL, Qt::QueuedConnection);
#else
    connect(&Pipeline::instance(), &Pipeline::sendImage,
            this, &MainWindow::updateImage, Qt::QueuedConnection);
#endif
    Pipeline::instance().setFuncName("color");
    camera->setProcess([=](const QVideoFrame &frame) {
        Pipeline::instance().dispatch(frame);
    });
    /* pipeline */
    Pipeline::instance().start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::capture()
{
    if (readyCapture.load()) {
        return;
    }
    readyCapture.store(1);
    return;
}

void MainWindow::updateImage(const QImage &img)
{
    QPixmap pixmap = QPixmap::fromImage(img);
    if (readyCapture.load()) {
        QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
#ifdef Q_OS_ANDROID
        QString picturePath = Configuration::instance().getPicturePath();
        QString path = QString("%1/%2%3").arg(picturePath).arg(dateTime).arg(".jpeg");
#else
        QString path = QString("./%1%2").arg(dateTime).arg(".jpeg");
#endif
        pixmap.save(path);
        statusBar()->showMessage(path);
        readyCapture.store(0);
    }
    ui->videoLabel->setPixmap(pixmap.scaled(ui->videoLabel->size()));
    return;
}

void MainWindow::updateGL(int w, int h, unsigned char *data)
{
    if (readyCapture.load()) {
        QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
#ifdef Q_OS_ANDROID
        QString picturePath = Configuration::instance().getPicturePath();
        QString path = QString("%1/%2%3").arg(picturePath).arg(dateTime).arg(".jpeg");
#else
        QString path = QString("./%1%2").arg(dateTime).arg(".jpeg");
#endif
        QImage img(data, w, h, QImage::Format_ARGB32);
        img.save(path);
        statusBar()->showMessage(path);
        readyCapture.store(0);
    }
    ui->openGLWidget->setFrame(w, h, data);
    return;
}

void MainWindow::setPage(int index)
{
    if (pageID == index) {
        return;
    }
    switch (index) {
    case PAGE_OPENGL_VIDEO:
        ui->stackedWidget->setCurrentIndex(PAGE_OPENGL_VIDEO);
        ui->openGLWidget->show();
        break;
    case PAGE_VIDEO:
        ui->stackedWidget->setCurrentIndex(PAGE_VIDEO);
        ui->videoLabel->show();
        break;
    case PAGE_SETTINGS:
        ui->stackedWidget->setCurrentIndex(PAGE_SETTINGS);
        ui->settingWidget->show();
        break;
    default:
        break;
    }
    pageID = index;
    return;
}

void MainWindow::startRecord()
{
    QString videoPath = Configuration::instance().getVideoPath();
    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
#ifdef Q_OS_ANDROID
    QString fileName = QString("%1/%2%3").arg(videoPath).arg(dateTime).arg(".mp4");
#else
    QString fileName = QString("./%1%2").arg(dateTime).arg(".mp4");
#endif
    Recorder::instance().start(IMG_WIDTH, IMG_HEIGHT, AV_PIX_FMT_RGB24, fileName.toStdString());
    statusBar()->showMessage("Recording");
    return;
}

void MainWindow::stopRecord()
{
    Recorder::instance().stop();
    statusBar()->showMessage("Record completed.");
    return;
}

void MainWindow::setProcessFunc(const QString &funcName)
{
    Pipeline::instance().setFuncName(funcName.toStdString());
    statusBar()->showMessage(funcName);
    return;
}

bool MainWindow::event(QEvent *ev)
{
    switch (ev->type()) {
    case QTouchEvent::TouchBegin:
    case QTouchEvent::TouchUpdate:
        camera->searchAndLock();
        ev->accept();
        update();
        break;
    case QTouchEvent::TouchEnd:
        camera->unlock();
        update();
        break;
    default:
        break;
    }
    return QMainWindow::event(ev);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_ANDROID
    if (readyQuit == false) {
        event->ignore();
        return;
    }
#endif
    return QMainWindow::closeEvent(event);

}

void MainWindow::createMenu()
{
    /* quit */
    QAction *quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, this, [=](){
        readyQuit = true;
        qApp->quit();
    });
    ui->menu->addAction(quitAction);
    /* settings */
    QAction *settingAction = new QAction(tr("Settings"), this);
    connect(settingAction, &QAction::triggered, this, [=](){setPage(1);});
    ui->menu->addAction(settingAction);
    /* camera */
    QAction *cameraAction = new QAction(tr("Camera"), this);
    connect(cameraAction, &QAction::triggered, this, [=](){setPage(2);});
    ui->menu->addAction(cameraAction);
    /* method */
    QAction *canny = new QAction(tr("canny"), this);
    connect(canny, &QAction::triggered, this, [=](){setProcessFunc("canny");});
    ui->menu->addAction(canny);
    QAction *sobel = new QAction(tr("sobel"), this);
    connect(sobel, &QAction::triggered, this, [=](){setProcessFunc("sobel");});
    ui->menu->addAction(sobel);
    QAction *laplace = new QAction(tr("laplace"), this);
    connect(laplace, &QAction::triggered, this, [=](){setProcessFunc("laplace");});
    ui->menu->addAction(laplace);
    QAction *haarcascade = new QAction(tr("haarcascade"), this);
    connect(haarcascade, &QAction::triggered, this, [=](){setProcessFunc("haarcascade");});
    ui->menu->addAction(haarcascade);
    QAction *yolov4 = new QAction(tr("yolov4"), this);
    connect(yolov4, &QAction::triggered, this, [=](){setProcessFunc("yolov4");});
    ui->menu->addAction(yolov4);
    QAction *yolov5 = new QAction(tr("yolov5"), this);
    connect(yolov5, &QAction::triggered, this, [=](){setProcessFunc("yolov5");});
    ui->menu->addAction(yolov5);
    QAction *yolov7 = new QAction(tr("yolov7"), this);
    connect(yolov7, &QAction::triggered, this, [=](){setProcessFunc("yolov7");});
    ui->menu->addAction(yolov7);
    QAction *linear = new QAction(tr("color"), this);
    connect(linear, &QAction::triggered, this, [=](){setProcessFunc("color");});
    ui->menu->addAction(linear);
    /* capture */
    QAction *captureAction = new QAction(tr("capture"), this);
    connect(captureAction, &QAction::triggered, this, &MainWindow::capture);
    ui->menu->addAction(captureAction);
    /* record */
    QAction *startRecordAction = new QAction(tr("start record"), this);
    connect(startRecordAction, &QAction::triggered, this, &MainWindow::startRecord);
    ui->menu->addAction(startRecordAction);
    QAction *stopRecordAction = new QAction(tr("stop record"), this);
    connect(stopRecordAction, &QAction::triggered, this, &MainWindow::stopRecord);
    ui->menu->addAction(stopRecordAction);
    return;
}

