#include "LED.h"
#include "plog/Log.h"
#include <fcntl.h>

LED::LED(std::string _dev_path) : is_on(false), dev_path(_dev_path)
{
    this->fd = open(dev_path.c_str(), O_RDWR);
    if (this->fd < 0)
    {
        PLOGF << "Failed to open LED device: " << this->dev_path;
        std::exit(1);
    }
    PLOGI << "LED device opened: " << this->dev_path;
}

LED::~LED()
{
    close(this->fd);
    PLOGI << "LED device closed: " << this->dev_path;
}

void LED::on()
{
    if (this->is_on)
        return;

    this->is_on = true;
    uint8_t val = 1;
    write(this->fd, &val, 1);
    PLOGI << "LED turned on: " << this->dev_path;
}

void LED::off()
{
    if (!this->is_on)
        return;

    this->is_on = false;
    uint8_t val = 0;
    write(this->fd, &val, 1);
    PLOGI << "LED turned off: " << this->dev_path;
}
