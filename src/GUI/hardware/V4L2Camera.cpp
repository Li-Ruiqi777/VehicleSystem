#include "V4L2Camera.h"
#include "plog/Log.h"

#include <cstdint>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/mman.h>

V4L2Camera::V4L2Camera(const std::string &_dev_path, uint32_t _width, uint32_t _height, uint32_t _frame_rate,
                       uint32_t _buffer_length)
    : dev_path(_dev_path), fd(-1), width(_width), height(_height), frame_rate(_frame_rate),
      is_capturing(false), buffer_length(_buffer_length), buffers(nullptr), frame_queue_length(_buffer_length)
{
    try
    {
        this->openDevice();
        this->setParameters();
        this->initBuffer();
        PLOGI << "V4L2Camera Init " << this->dev_path << " Success!";
    }
    catch (std::exception &e)
    {
        PLOGE << e.what();
    }
}

V4L2Camera::~V4L2Camera()
{
    this->stopCapture();
    this->deinitBuffer();
    this->closeDevice();
}

void V4L2Camera::startCapture()
{
    if (this->is_capturing == true)
        return;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0)
        throw std::runtime_error("start capture failed");
    this->is_capturing = true;
    this->capture_thread = std::thread(&V4L2Camera::dataCollectionLoop, this);
    PLOGI << "start capture";
}

void V4L2Camera::stopCapture()
{
    if (this->is_capturing == false)
        return;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
        throw std::runtime_error("start capture failed");
    this->is_capturing = false;
    this->capture_thread.join();
    PLOGI << "stop capture";
}

void V4L2Camera::openDevice()
{
    this->fd = open(this->dev_path.c_str(), O_RDWR);
    if (fd < 0)
        throw std::runtime_error("Failed to open device " + this->dev_path);

    v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
        throw std::runtime_error("Failed to query device capabilities");

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        throw std::runtime_error("Device is not a video capture device");

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
        throw std::runtime_error("Device does not support streaming I/O");

    PLOGI << "V4L2Camera Open " << this->dev_path;
}

void V4L2Camera::closeDevice()
{
    if (this->fd < 0)
        return;
    close(this->fd);
    this->fd = -1;
    PLOGI << "V4L2Camera Close" << this->dev_path;
}

void V4L2Camera::getParameters()
{
    v4l2_fmtdesc format_desc;    // 像素格式描述
    v4l2_frmsizeenum frame_size; // 分辨率描述
    v4l2_frmivalenum frame_val;  // 帧率描述

    format_desc.index = 0;
    format_desc.type = frame_size.type = frame_val.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &format_desc) == 0)
    {
        PLOGI << "Pixel Format:" << format_desc.description;
        frame_size.index = 0;
        frame_size.pixel_format = format_desc.pixelformat;
        frame_val.pixel_format = format_desc.pixelformat;
        // 获得该像素格式的分辨率
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) == 0)
        {
            PLOGI << "-Resolution:" << frame_size.discrete.width << "x" << frame_size.discrete.height;
            frame_val.index = 0;
            frame_val.width = frame_size.discrete.width;
            frame_val.height = frame_size.discrete.height;
            // 获得该分辨率的帧率
            while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_val) == 0)
            {
                PLOGI << "-Frame Rate:" << frame_val.discrete.denominator / frame_val.discrete.numerator;
                frame_val.index++;
            }
            frame_size.index++;
        }
        format_desc.index++;
    }
}

void V4L2Camera::setParameters()
{
    // 设置帧格式
    v4l2_format frame_format = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .fmt =
            {
                .pix =
                    {
                        .width = this->width,
                        .height = this->height,
                        .pixelformat = V4L2_PIX_FMT_YUYV,
                    },
            },
    };
    if (ioctl(fd, VIDIOC_S_FMT, &frame_format) < 0)
        throw std::runtime_error("Failed to set frame format");

    // 设置帧率
    v4l2_streamparm stream_param = {.type = V4L2_BUF_TYPE_VIDEO_CAPTURE};
    if (ioctl(fd, VIDIOC_G_PARM, &stream_param) < 0)
        throw std::runtime_error("Failed to get stream parameters");

    if (!(stream_param.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
        throw std::runtime_error("Device does not support setting frame rate");

    stream_param.parm.capture.timeperframe.numerator = 1;
    stream_param.parm.capture.timeperframe.denominator = this->frame_rate;
    if (ioctl(fd, VIDIOC_S_PARM, &stream_param) < 0)
        throw std::runtime_error("Failed to set stream parameters");
}

void V4L2Camera::initBuffer()
{
    this->buffers = std::unique_ptr<BufferInfo[]>(new BufferInfo[this->buffer_length]);
    // 在内核申请缓冲区·
    v4l2_requestbuffers req_buf;
    req_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_buf.memory = V4L2_MEMORY_MMAP;
    req_buf.count = this->buffer_length;
    if (ioctl(fd, VIDIOC_REQBUFS, &req_buf) < 0)
        throw std::runtime_error("Failed to request buffer");

    v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    for (int i = 0; i < this->buffer_length; i++)
    {
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0)
            throw std::runtime_error("Failed to query buffer");
        
        // 映射缓冲区到用户空间
        this->buffers[i].length = buf.length;
        this->buffers[i].start = (uint8_t *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (this->buffers[i].start == MAP_FAILED)
            throw std::runtime_error("Failed to map buffer");
        
        // 入队
        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)
            throw std::runtime_error("Failed to queue buffer");
    }
}

void V4L2Camera::deinitBuffer()
{
    if (this->buffers == nullptr)
        return;
    for (int i = 0; i < this->buffer_length; i++)
    {
        v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        // 出队
        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
        {
            PLOGE << "Failed to dequeue buffer";
            // throw std::runtime_error("Failed to dequeue buffer");
        }
        // 解除映射
        munmap(this->buffers[i].start, this->buffers[i].length);
        this->buffers[i].start = nullptr;
    }
}

void V4L2Camera::dataCollectionLoop()
{
    v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    while (this->is_capturing)
    {
        for (int i = 0; i < this->buffer_length; ++i)
        {
            // 出队
            if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
            {
                // throw std::runtime_error("Failed to dequeue buffer");
                PLOGE << "Failed to dequeue buffer";
                continue;
            }

            // 处理图像数据
            FrameData frame_data;
            frame_data.width = this->width;
            frame_data.height = this->height;
            frame_data.yuyv_data = std::shared_ptr<uint8_t>(new uint8_t[this->buffers[i].length], [](uint8_t *p) {delete[] p;});
            memccpy(frame_data.yuyv_data.get(), this->buffers[i].start, 0, this->buffers[i].length);

            {
                std::lock_guard<std::mutex> lock(this->frame_queue_mutex);
                this->frame_queue.push(frame_data);
                // PLOGI << "get one frame";

                // 如果队列超过最大长度，移除最旧的帧
                if (this->frame_queue.size() > this->frame_queue_length)
                    this->frame_queue.pop();
            }

            // 入队
            if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)
                throw std::runtime_error("Failed to queue buffer");
        }
    }
}

FrameData V4L2Camera::getFrame()
{
    std::lock_guard<std::mutex> lock(this->frame_queue_mutex);
    if (this->frame_queue.empty())
    {
        FrameData temp;
        temp.yuyv_data = nullptr;
        return temp;
    }
        
    FrameData frame_data = this->frame_queue.front();
    this->frame_queue.pop();
    return frame_data;
}