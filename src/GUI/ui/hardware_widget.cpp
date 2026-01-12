#include "hardware_widget.h"
#include "ui_hardware_widget.h"
#include "plog/Log.h"

#include "fcntl.h"
#include "unistd.h"
#include <cstdlib>
#include <memory>
#include <QString>

HardWareWidget::HardWareWidget(QWidget *parent) : QWidget(parent), ui(new Ui::HardWareWidget)
{
    ui->setupUi(this);

    // 初始化 LED
    this->led = std::unique_ptr<LED>(new LED("/dev/led"));
    this->led->on();

    // 初始化 AP3216C 传感器
    try
    {
        this->ap3216c = std::unique_ptr<AP3216C>(new AP3216C("/dev/ap3216c"));
    }
    catch (const std::exception &e)
    {
        PLOGE << "Failed to initialize AP3216C: " << e.what();
        this->ap3216c = nullptr;
    }

    // 创建定时器用于定期读取传感器数据
    this->sensor_timer = new QTimer(this);
    connect(this->sensor_timer, &QTimer::timeout, this, &HardWareWidget::updateSensorData);
    
    // 每 500ms 更新一次传感器数据
    this->sensor_timer->start(500);
}

HardWareWidget::~HardWareWidget()
{
    if (this->sensor_timer)
    {
        this->sensor_timer->stop();
        delete this->sensor_timer;
    }
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

void HardWareWidget::updateSensorData()
{
    if (!this->ap3216c || !this->ap3216c->isOpen())
    {
        return;
    }

    SensorData data = this->ap3216c->readData();
    
    // 更新 UI 标签
    ui->label_ps->setText(QString::number(data.ps_data));
    ui->label_als->setText(QString::number(data.als_data));
    ui->label_ir->setText(QString::number(data.ir_data));
}
