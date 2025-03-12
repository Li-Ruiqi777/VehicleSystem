#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>

namespace Ui
{
    class VideoWidget;
}

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

private:
    Ui::VideoWidget *ui;
};

#endif // VIDEO_WIDGET_H
