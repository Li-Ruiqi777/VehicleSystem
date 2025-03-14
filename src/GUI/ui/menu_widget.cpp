#include "menu_widget.h"
#include "ui_menu_widget.h"
#include "common.h"

#include "plog/Log.h"

#include "fcntl.h"
#include "linux/input.h"
#include "unistd.h"

MenuWidget::MenuWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MenuWidget)
{
    ui->setupUi(this);

    connect(ui->buttonGroup,
            static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), this,
            &MenuWidget::on_buttonGroup_buttonClicked);

    this->initBoardButton();
}

MenuWidget::~MenuWidget()
{
    delete ui;
    if(this->btn_fd >= 0)
        ::close(this->btn_fd);   
}

void MenuWidget::on_buttonGroup_buttonClicked(QAbstractButton *button)
{
    QPushButton *btn = dynamic_cast<QPushButton *>(button);
    int idx = btn->property("idx").toInt();
    emit changePage(idx);
}

void MenuWidget::initBoardButton()
{
    this->btn_fd = open("/dev/input/event2", O_RDONLY);
    if (this->btn_fd < 0)
    {
        PLOG_FATAL << "open KEY0 failed";
        std::exit(1);
    }

    this->btn_thread = std::unique_ptr<std::thread>(new std::thread(
        [this]()
        {
            while (1)
            {
                input_event ev;
                if (read(this->btn_fd, &ev, sizeof(ev)) != sizeof(ev))
                {
                    PLOG_FATAL << "read KEY0 event failed";
                    continue;
                }
                    
                if (ev.type == EV_KEY && ev.code == KEY_0 && ev.value == 1)
                {
                    emit changePage(static_cast<int>(PageEnum::MenuPage));
                    PLOGI << "Button 0 pressed";
                }
            }
        }));
    this->btn_thread->detach();
}