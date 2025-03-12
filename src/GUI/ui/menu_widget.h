#ifndef MENU_WIDGET_H
#define MENU_WIDGET_H

#include <QWidget>
#include <QAbstractButton>

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
    Ui::MenuWidget *ui;
};

#endif // MENU_WIDGET_H
