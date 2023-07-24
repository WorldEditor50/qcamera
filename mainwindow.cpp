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
    /* android permission */
    requestPermission();
    /* ffmepg network */
    RtspPublisher::instance().enableNetwork();
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
    /* back */
    connect(ui->imageViewer, &ImageViewer::back, this, [=](){
        setPage(PAGE_VIDEO);
    });
    connect(ui->settingWidget, &Setting::back, this, [=](){
        setPage(PAGE_VIDEO);
    });

    /* camera */
    connect(this, &MainWindow::send, this, &MainWindow::updateImage, Qt::QueuedConnection);
    Camera::infoList = QCameraInfo::availableCameras();
    /* start camera */
    camera = new Camera(this);
    ui->settingWidget->setDevice(camera);
#if USE_OPENGL
    connect(&Pipeline::instance(), &Pipeline::sendGlImage,
            this, &MainWindow::updateGL, Qt::QueuedConnection);
#else
    connect(&Pipeline::instance(), &Pipeline::sendImage,
            this, &MainWindow::updateImage, Qt::QueuedConnection);
#endif
    Pipeline::instance().setFunc(PROCESS_COLOR);
    camera->setProcess([=](const QVideoFrame &frame) {
        Pipeline::instance().dispatch(frame);
    });
    /* pipeline */
    Pipeline::instance().start();
    /* load model */
    connect(this, &MainWindow::modelReady, this, &MainWindow::launch, Qt::QueuedConnection);
    QtConcurrent::run([=](){
        bool ret = true;
#ifdef Q_OS_ANDROID
        QString modelPath = Configuration::instance().getModelPath();
        ret = Cascade::instance().load(modelPath.toStdString());
#else
        QString modelPath = "D:/home/MyProject/qcamera/ncnn/models";
        //ret = Cascade::instance().load("");
#endif
        ret = Yolov7::instance().load(QString("%1/yolov7-tiny").arg(modelPath).toStdString());
        ret = Yolov5::instance().load(QString("%1/yolov5s_6.0").arg(modelPath).toStdString());
        if (ret == false) {
            statusBar()->showMessage("Failed to load model.");
        } else {
            statusBar()->showMessage("load model completed.");
        }
        emit modelReady();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::requestPermission()
{
#ifdef Q_OS_ANDROID
    QStringList permissions = {
        "android.permission.READ_EXTERNAL_STORAGE",
        "android.permission.WRITE_EXTERNAL_STORAGE",
        "android.permission.CAMERA",
        "android.permission.INTERNET"
    };
    for (QString &permission : permissions) {
        QtAndroid::PermissionResult result = QtAndroid::checkPermission(permission);
        if (result == QtAndroid::PermissionResult::Denied) {
            QtAndroid::requestPermissionsSync(QStringList{permission});
        }
    }
#endif
    return;
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
        /* display image */
        setPage(PAGE_IMAGE);
        ui->imageViewer->display(pixmap);
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
    case PAGE_IMAGE:
        ui->stackedWidget->setCurrentIndex(PAGE_IMAGE);
        ui->imageViewer->show();
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
    QString format = Configuration::instance().getVideoFormat();
#ifdef Q_OS_ANDROID
    QString fileName = QString("%1/%2.%3").arg(videoPath).arg(dateTime).arg(format);
#else
    QString fileName = QString("./%1.%2").arg(dateTime).arg(format);
#endif
    Recorder::instance().start(IMG_WIDTH, IMG_HEIGHT, AV_PIX_FMT_RGB24,
                               format.toStdString(), fileName.toStdString());
    statusBar()->showMessage("Recording");
    return;
}

void MainWindow::stopRecord()
{
    Recorder::instance().stop();
    statusBar()->showMessage("Record completed.");
    return;
}

void MainWindow::startStream()
{
    QString url = ui->settingWidget->getStreamURL();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Notice", "empty rtmp url", QMessageBox::Ok);
        return;
    }
    RtspPublisher::instance().start(IMG_WIDTH, IMG_HEIGHT, url.toStdString());
    statusBar()->showMessage("START STREAMING");
    return;
}

void MainWindow::stopStream()
{
    RtspPublisher::instance().stop();
    statusBar()->showMessage("STOP STREAMING");
    return;
}

void MainWindow::launch()
{
    camera->start(0);
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
    Pipeline::instance().stop();
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
    connect(settingAction, &QAction::triggered, this, [=](){setPage(PAGE_SETTINGS);});
    ui->menu->addAction(settingAction);
    /* method */
    QAction *canny = new QAction(tr("canny"), this);
    connect(canny, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_CANNY);
        statusBar()->showMessage("canny");
    });
    ui->menu->addAction(canny);
    QAction *sobel = new QAction(tr("sobel"), this);
    connect(sobel, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_SOBEL);
        statusBar()->showMessage("sobel");
    });
    ui->menu->addAction(sobel);
    QAction *laplace = new QAction(tr("laplace"), this);
    connect(laplace, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_LAPLACE);
        statusBar()->showMessage("laplace");
    });
    ui->menu->addAction(laplace);
    QAction *haarcascade = new QAction(tr("haarcascade"), this);
    connect(haarcascade, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_CASCADE);
        statusBar()->showMessage("haarcascade");
    });
    ui->menu->addAction(haarcascade);
    QAction *yolov5 = new QAction(tr("yolov5"), this);
    connect(yolov5, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_YOLOV5);
        statusBar()->showMessage("yolov5");
    });
    ui->menu->addAction(yolov5);
    QAction *yolov7 = new QAction(tr("yolov7"), this);
    connect(yolov7, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_YOLOV7);
        statusBar()->showMessage("yolov7");
    });
    ui->menu->addAction(yolov7);

    QAction *opticalFlow = new QAction(tr("opticalFlow"), this);
    connect(opticalFlow, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_OPTICALFLOW);
        statusBar()->showMessage("opticalFlow");
    });
    ui->menu->addAction(opticalFlow);

    QAction *linear = new QAction(tr("color"), this);
    connect(linear, &QAction::triggered, this, [=](){
        Pipeline::instance().setFunc(PROCESS_COLOR);
        statusBar()->showMessage("color");
    });
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
    /* streaming */
    QAction *startStreamAction = new QAction(tr("start streaming"), this);
    connect(startStreamAction, &QAction::triggered, this, &MainWindow::startStream);
    ui->menu->addAction(startStreamAction);
    QAction *stopStreamAction = new QAction(tr("stop streaming"), this);
    connect(stopStreamAction, &QAction::triggered, this, &MainWindow::stopStream);
    ui->menu->addAction(stopStreamAction);
    return;
}

