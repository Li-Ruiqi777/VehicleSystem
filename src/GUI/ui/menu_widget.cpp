#include "menu_widget.h"
#include "ui_menu_widget.h"
#include <QDebug>

MenuWidget::MenuWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MenuWidget)
{
    ui->setupUi(this);

    connect(ui->buttonGroup, static_cast<void(QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked),
        this, &MenuWidget::on_buttonGroup_buttonClicked);

}

MenuWidget::~MenuWidget()
{
    delete ui;
}

void MenuWidget::on_buttonGroup_buttonClicked(QAbstractButton *button)
{
    QPushButton *btn = dynamic_cast<QPushButton *>(button);
    int idx = btn->property("idx").toInt();
    qDebug() << "Button " << idx << " clicked";
    emit changePage(idx);
}