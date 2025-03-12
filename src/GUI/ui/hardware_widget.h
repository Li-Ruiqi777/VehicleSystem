#ifndef HARDWARE_WIDGET_H
#define HARDWARE_WIDGET_H

#include <QWidget>

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

private:
    Ui::HardWareWidget *ui;
};

#endif // HARDWARE_WIDGET_H
