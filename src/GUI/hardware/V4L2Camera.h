#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <sys/types.h>
#include <thread>

#include "frame_data.h"

struct BufferInfo
{
    uint8_t *start;
    size_t length;
};

class V4L2Camera
{
public:
    V4L2Camera(const std::string &_dev_path, uint32_t _width, uint32_t _height, uint32_t _frame_rate,
               uint32_t _buffer_length);
    ~V4L2Camera();

    void startCapture();
    void stopCapture();
    void getParameters();
    FrameData getFrame();

private:
    void openDevice();
    void closeDevice();
    void setParameters();
    void initBuffer();
    void deinitBuffer();
    void dataCollectionLoop();

    std::string dev_path;
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t frame_rate;
    bool is_capturing;
    uint32_t buffer_length;
    std::unique_ptr<BufferInfo[]> buffers;

    std::queue<FrameData> frame_queue;
    uint32_t frame_queue_length;
    std::mutex frame_queue_mutex;
    
    std::thread capture_thread;
};