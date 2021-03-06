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
#include "camera.h"
#include "imageprocess.h"
#include "setting.h"
#include "pipeline.h"
#include "configuration.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum PageID {
        PAGE_VIDEO = 0,
        PAGE_SETTINGS
    };
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void send(const QImage &img);
public slots:
    void capture();
    void updateImage(const QImage &img);
    void setPage(int index);
    void startRecord();
    void stopRecord();
    void setProcessFunc(const QString &funcName);
protected:
    bool event(QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;
private:
    void createMenu();
private:
    Ui::MainWindow *ui;
    int pageID;
    bool readyQuit;
    QLabel *videoLabel;
    Setting *setting;
    Camera *camera;
    QAtomicInt readyCapture;
};
#endif // MainWindow
