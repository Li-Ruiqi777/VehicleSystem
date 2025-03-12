#include "hardware_widget.h"
#include "ui_hardware_widget.h"

HardWareWidget::HardWareWidget(QWidget *parent) : QWidget(parent), ui(new Ui::HardWareWidget)
{
    ui->setupUi(this);
}

HardWareWidget::~HardWareWidget()
{
    delete ui;
}
