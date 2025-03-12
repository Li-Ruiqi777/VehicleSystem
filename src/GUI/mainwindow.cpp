#include "mainwindow.h"
#include "ui/common.h"
#include "ui/menu_widget.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(static_cast<int>(PageEnum::MenuPage));

    connect(ui->page_menu, &MenuWidget::changePage, this, &MainWindow::onChangePage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onChangePage(int idx)
{
    std::cout << "onChangePage: " << idx << std::endl;
    if (idx < static_cast<int>(PageEnum::HardWareControlPage) || idx > static_cast<int>(PageEnum::MusicPage))
        return;
    ui->stackedWidget->setCurrentIndex(idx);
}