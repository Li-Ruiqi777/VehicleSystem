#ifndef HARDWARE_WIDGET_H
#define HARDWARE_WIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>
#include "LED.h"
#include "AP3216C.h"

namespace Ui
{
    class HardWareWidget;
}

class HardWareWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HardWareWidget(QWidget *parent = nullptr);
    ~HardWareWidget();

private slots:
    void on_btn_led_on_clicked();
    void on_btn_led_off_clicked();
    void updateSensorData();

private:
    Ui::HardWareWidget *ui;
    std::unique_ptr<LED> led;
    std::unique_ptr<AP3216C> ap3216c;
    QTimer *sensor_timer;

};

#endif // HARDWARE_WIDGET_H
