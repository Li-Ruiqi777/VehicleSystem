#pragma once
#include <string>

class LED
{
public:
    LED(std::string _dev_path);
    ~LED();
    void on();
    void off();

private:
    int fd;
    bool is_on;
    std::string dev_path;
};