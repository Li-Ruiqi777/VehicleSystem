#ifndef MENU_WIDGET_H
#define MENU_WIDGET_H

#include <QWidget>
#include <QAbstractButton>

#include <thread>
#include <memory>

namespace Ui
{
    class MenuWidget;
}

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

    void on_buttonGroup_buttonClicked(QAbstractButton *button);
    
signals:
    void changePage(int idx);

private:
    void initBoardButton();

    Ui::MenuWidget *ui;
    int btn_fd;
    std::unique_ptr<std::thread> btn_thread;
};

#endif // MENU_WIDGET_H
