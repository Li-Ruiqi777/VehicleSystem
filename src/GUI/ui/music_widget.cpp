#include "music_widget.h"
#include "ui_music_widget.h"

MusicWidget::MusicWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MusicWidget)
{
    ui->setupUi(this);
}

MusicWidget::~MusicWidget()
{
    delete ui;
}
