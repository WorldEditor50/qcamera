#ifndef SINGLEAPP_H
#define SINGLEAPP_H

#include <QObject>
#include <QApplication>
#include <QLocalServer>
#include <QWidget>
#include <QLocalSocket>
#include <QFileInfo>
#include <QDebug>

#define TIME_OUT 500

class SingleApp : public QApplication
{
    Q_OBJECT
protected:
    bool runFlag;
    QLocalServer *localServer;
    QString serverName;
    QWidget *w;
public:
    SingleApp(int &argc, char **argv)
        : QApplication(argc, argv)
        , runFlag(false)
        , localServer(nullptr)
        , w(nullptr)
    {
        serverName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(TIME_OUT)) {
            runFlag = true;
        } else {
            localServer = new QLocalServer(this);
            connect(localServer, &QLocalServer::newConnection, this, &SingleApp::addConnection);
            if (!localServer->listen(serverName)) {
                // 此时监听失败，可能是程序崩溃时,残留进程服务导致的,移除之
                if (localServer->serverError() == QAbstractSocket::AddressInUseError) {
                    QLocalServer::removeServer(serverName);
                    localServer->listen(serverName);
                }
            }
            runFlag = false;
        }

    }
    bool isRunning() const {return runFlag;}
    void setWindow(QWidget *w_){ w = w_; }
private slots:
    void addConnection()
    {
        QLocalSocket *socket = localServer->nextPendingConnection();
        if (socket != nullptr) {
            socket->waitForReadyRead(2*TIME_OUT);
            delete socket;
            if (w != nullptr) {
                w->show();
                w->raise();
                w->activateWindow();
            }
        }
        return;
    }
};

#endif // SINGLEAPP_H
