#include "video_widget.h"
#include "ui_video_widget.h"

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent), ui(new Ui::VideoWidget)
{
    ui->setupUi(this);
}

VideoWidget::~VideoWidget()
{
    delete ui;
}
