#pragma once
#include <string>
#include <sys/ioctl.h>

#define LED_MAGIC 'L'
#define LED_ON    _IO(LED_MAGIC, 0)
#define LED_OFF   _IO(LED_MAGIC, 1)

class LED
{
public:
    LED(const std::string &_dev_path);
    ~LED();
    void on();
    void off();

private:
    void openDevice();
    void closeDevice();

    int fd;
    bool is_on;
    std::string dev_path;
};