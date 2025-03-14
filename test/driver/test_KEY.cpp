#include <iostream>
#include <string>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <linux/input.h>

#include <memory>

const std::string KEY_DEVICE = "/dev/input/event2";

int main()
{
    // 打开设备文件
    struct input_event event;

    int fd = open(KEY_DEVICE.c_str(), O_RDONLY);
    if (fd < 0)
    {
        perror("event not exists!");
        exit(-1);
    }

    while (1)
    {
        std::cout << "waiting for key event..." << std::endl;
        if (read(fd, &event, sizeof(struct input_event)) != sizeof(struct input_event))
        {
            perror("read failed!");
            exit(-1);
        }

        if (event.type == EV_KEY)
        {
            switch (event.value)
            {
                case 0:
                    printf("code<%d>: 松开\n", event.code);
                    break;
                case 1:
                    printf("code<%d>: 按下\n", event.code);
                    break;
                case 2:
                    printf("code<%d>: 长按\n", event.code);
                    break;
            }
        }
    }
}
