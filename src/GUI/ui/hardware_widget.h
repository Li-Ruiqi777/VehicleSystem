#ifndef HARDWARE_WIDGET_H
#define HARDWARE_WIDGET_H

#include <QWidget>

#include <memory>
#include "LED.h"

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

private:
    Ui::HardWareWidget *ui;
    std::unique_ptr<LED> led;

};

#endif // HARDWARE_WIDGET_H
