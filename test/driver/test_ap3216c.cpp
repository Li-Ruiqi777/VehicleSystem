#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>

const std::string AP3216C_DEVICE = "/dev/ap3216c";

int main()
{
    int fd = open(AP3216C_DEVICE.c_str(), O_RDWR);
    if (fd < 0)
    {
        std::cout << "open ap3216c failed" << std::endl;
        return -1;
    }
    uint16_t data[3];
    while (1)
    {
        read(fd, data, sizeof(data));
        std::cout << "ir data: " << data[0] << std::endl;
        std::cout << "als data: " << data[1] << std::endl;
        std::cout << "ps data: " << data[2] << std::endl << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    close(fd);
}