#ifndef BACKUPVIEW_WIDGET_H
#define BACKUPVIEW_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <memory>
#include "V4L2Camera.h"

namespace Ui
{
    class BackupviewWidget;
}

class BackupviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BackupviewWidget(QWidget *parent = nullptr);
    ~BackupviewWidget();

private slots:
    void on_btn_start_clicked();
    void on_btn_stop_clicked();
    void on_btn_info_clicked();

private:
    Ui::BackupviewWidget *ui;
    std::unique_ptr<V4L2Camera> camera;
    std::unique_ptr<QTimer> update_img_timer;
};

#endif // BACKUPVIEW_WIDGET_H
