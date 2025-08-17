#include <fcntl.h>
#include <iostream>
#include <linux/watchdog.h>
#include <string>
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>


const std::string WDG_DEVICE = "/dev/watchdog";

int main(int argc, char *argv[])
{
    struct watchdog_info info;
    int timeout;
    int time;
    int fd;
    int op;
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <timeout>\n";
        fprintf(stderr, "usage: %s <timeout>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // 打开看门狗 
    fd = open(WDG_DEVICE.c_str(), O_RDWR);
    if (0 > fd)
    {
        fprintf(stderr, "open error: %s: %s\n", WDG_DEVICE.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 打开之后看门狗计时器会开启、先停止它 
    op = WDIOS_DISABLECARD;
    if (0 > ioctl(fd, WDIOC_SETOPTIONS, &op))
    {
        fprintf(stderr, "ioctl error: WDIOC_SETOPTIONS: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 设置超时时间 
    timeout = atoi(argv[1]);
    if (1 > timeout)
        timeout = 1;
    printf("timeout: %ds\n", timeout);
    if (0 > ioctl(fd, WDIOC_SETTIMEOUT, &timeout))
    {
        fprintf(stderr, "ioctl error: WDIOC_SETTIMEOUT: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 开启看门狗计时器 
    op = WDIOS_ENABLECARD;
    if (0 > ioctl(fd, WDIOC_SETOPTIONS, &op))
    {
        fprintf(stderr, "ioctl error: WDIOC_SETOPTIONS: %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 喂狗 
    time = (timeout * 1000 - 100) * 1000; // 喂狗时间设置 us 微秒、在超时时间到来前 100ms 喂狗
    while(1)
    {
        usleep(time);
        ioctl(fd, WDIOC_KEEPALIVE, NULL);
        std::cout << "feed dog\n";
    }
}