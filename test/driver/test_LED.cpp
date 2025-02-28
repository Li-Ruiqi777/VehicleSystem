#include <iostream>
#include <string>
#include <fcntl.h>
#include <thread>
#include <unistd.h>

const std::string LED_DEVICE = "/dev/led";

void LED_Status_Convert(uint8_t nLedStatus)
{
    int nFd;
    uint8_t nVal;

    std::cout << "led write:" << nLedStatus << std::endl;
    nFd = open(LED_DEVICE.c_str(), O_RDWR | O_NDELAY);
    if(nFd != -1)
    {
        nVal = nLedStatus;
        write(nFd, &nVal, 1);  //将数据写入LED
        close(nFd);
    }
}

int main()
{
    LED_Status_Convert(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LED_Status_Convert(0);
    return 0;
}