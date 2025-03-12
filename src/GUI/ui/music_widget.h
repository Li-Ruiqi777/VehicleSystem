#ifndef MUSIC_WIDGET_H
#define MUSIC_WIDGET_H

#include <QWidget>

namespace Ui
{
    class MusicWidget;
}

class MusicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicWidget(QWidget *parent = nullptr);
    ~MusicWidget();

private:
    Ui::MusicWidget *ui;
};

#endif // MUSIC_WIDGET_H
