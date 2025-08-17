#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <string>
// 用户空间必须用这个ioctl.h
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <chrono>

const std::string LED_DEVICE = "/dev/led";

#define LED_MAGIC 'L'
#define LED_ON    _IO(LED_MAGIC, 0)
#define LED_OFF   _IO(LED_MAGIC, 1)

int main()
{
    int fd = open(LED_DEVICE.c_str(), O_RDWR);
    while (1)
    {
        ioctl(fd, LED_ON);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ioctl(fd, LED_OFF);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    close(fd);
}