# 定义编译器
CXX = arm-linux-gnueabihf-g++
INCLUDES = /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/include

# 定义编译选项
CXXFLAGS = -g -Wall -std=c++11 -pthread -I$(INCLUDES)

# 定义目标路径
TARGET_DIR = /home/lrq/linux/nfs/qtrootfs/test_driver

# 获取当前目录下的所有cpp文件
SRCS = $(wildcard *.cpp)

# 将cpp文件转换为可执行文件名
EXES = $(patsubst %.cpp,%,$(SRCS))

# 默认目标
all: $(EXES)

# 为每个可执行文件定义规则
$(EXES): %: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ && \
	mv $@ $(TARGET_DIR)

# 清理生成的文件
clean:
	rm -f $(TARGET_DIR)/*

# 确保clean是一个伪目标
.PHONY: clean
