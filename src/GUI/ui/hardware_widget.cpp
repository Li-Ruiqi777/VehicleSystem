#include "hardware_widget.h"
#include "ui_hardware_widget.h"
#include "plog/Log.h"

#include "fcntl.h"
#include "unistd.h"
#include <cstdlib>
#include <memory>

HardWareWidget::HardWareWidget(QWidget *parent) : QWidget(parent), ui(new Ui::HardWareWidget)
{
    ui->setupUi(this);

    this->led = std::unique_ptr<LED>(new LED("/dev/led"));
    this->led->on();
}

HardWareWidget::~HardWareWidget()
{
    delete ui;
}

void HardWareWidget::on_btn_led_on_clicked()
{
    this->led->on();
}


void HardWareWidget::on_btn_led_off_clicked()
{
    this->led->off();
}
