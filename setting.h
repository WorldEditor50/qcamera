#ifndef SETTING_H
#define SETTING_H

#include <QWidget>
#include <QFileDialog>
#include "camera.h"
#include "configuration.h"

namespace Ui {
class Setting;
}

class Setting : public QWidget
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();
    void setDevice(Camera *camera);
signals:
    void back();
public:
    void updateDevice(int index);
private:
    Ui::Setting *ui;
    Camera *camera;
};

#endif // SETTING_H
