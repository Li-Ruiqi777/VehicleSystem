#pragma once
#include <string>

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