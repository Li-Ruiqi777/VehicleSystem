#include "backupview_widget.h"
#include "ui_backupview_widget.h"

#include "plog/Log.h"

BackupviewWidget::BackupviewWidget(QWidget *parent) : QWidget(parent), ui(new Ui::BackupviewWidget)
{
    ui->setupUi(this);
    this->update_img_timer = std::unique_ptr<QTimer>(new QTimer(this));
    this->update_img_timer->setInterval(500);
    this->camera = std::unique_ptr<V4L2Camera>(new V4L2Camera("/dev/video1", 640, 480, 30, 3));

    connect(this->update_img_timer.get(), &QTimer::timeout, this, [&](){
        FrameData frame_data = this->camera->getFrame();
        if(frame_data.yuyv_data.get() == nullptr)
            return;
 
        QImage img(frame_data.get_rgb888_data(), frame_data.width, frame_data.height, QImage::Format_RGB888);
        auto resized_img = img.scaled(360, 272, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->ui->img_label->setPixmap(QPixmap::fromImage(resized_img));
    });
}

BackupviewWidget::~BackupviewWidget()
{
    delete ui;
}

void BackupviewWidget::on_btn_start_clicked()
{
    this->camera->startCapture();
    this->update_img_timer->start();
}


void BackupviewWidget::on_btn_stop_clicked()
{
    this->camera->stopCapture();
    this->update_img_timer->stop();
}


void BackupviewWidget::on_btn_info_clicked()
{
    this->camera->getParameters();
}

