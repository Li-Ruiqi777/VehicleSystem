#pragma once
#include <memory>

struct FrameData
{
    uint32_t width;
    uint32_t height;
    std::shared_ptr<uint8_t> yuyv_data; // 大小是 width * height * 2
    std::shared_ptr<uint8_t> rgb888_data; // 大小是 width * height * 3

    uint8_t* get_rgb888_data()
    {
        this->rgb888_data = std::shared_ptr<uint8_t>(new uint8_t[this->width * this->height * 3], std::default_delete<uint8_t[]>());
        // 将yuyv转换成rgb888
        for (int i = 0; i < this->width * this->height; i++)
        {
            uint8_t y = this->yuyv_data.get()[i * 2];
            uint8_t u = this->yuyv_data.get()[i * 2 + 1];
            uint8_t v = this->yuyv_data.get()[i * 2 + 3];
            int r = (int)(y + 1.402 * (v - 128));
            int g = (int)(y - 0.34414 * (u - 128) - 0.71414 * (v - 128));
            int b = (int)(y + 1.772 * (u - 128));
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            this->rgb888_data.get()[i * 3] = r;
            this->rgb888_data.get()[i * 3 + 1] = g;
            this->rgb888_data.get()[i * 3 + 2] = b;
        }
        return this->rgb888_data.get();
    }
};