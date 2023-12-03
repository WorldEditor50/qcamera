#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QFile>
#include <QMessageBox>
#include <QMainWindow>
#include <QAtomicInt>
#include <QMap>
#include <QLabel>
#include <QTouchEvent>
#include <QCloseEvent>
#include <QEvent>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QSurfaceFormat>
#include "camera.h"
#include "improcess/improcess.h"
#include "process_def.h"
#include "setting.h"
#include "recorder.h"
#include "rtmppublisher.h"
#include "rtsppublisher.h"
#include "pipeline.h"
#include "configuration.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum PageID {
        PAGE_OPENGL_VIDEO = 0,
        PAGE_SETTINGS,
        PAGE_VIDEO,
        PAGE_IMAGE
    };
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void send(const QImage &img);
    void modelReady();
public slots:
    void requestPermission();
    void capture();
    void updateImage(const QImage &img);
    void updateGL(int h, int w, unsigned char* data);
    void setPage(int index);
    void startRecord();
    void stopRecord();
    void startStream();
    void stopStream();
    void launch();
    void onRotate();
protected:
    bool event(QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;
private:
    void createMenu();
private:
    Ui::MainWindow *ui;
    int pageID;
    bool readyQuit;
    Camera *camera;
    QAtomicInt readyCapture;
};
#endif // MainWindow
