# VehicleSystem
基于嵌入式Linux的智能车载系统

## TODO
- [x] 引入PLOG库
- [x] 开发HardWareControl部分
- [x] 加入倒车影像回放部分
- [x] AP3216C的驱动
- [ ] LSM6DS3的驱动
- [ ] BMP280的驱动
- [ ] DHT11的驱动

## 遇到的问题
### 1.触摸屏不准

解决思路：
- 先用tslib测试一下，看看到底是Qt的问题还是触摸屏本身就不准
- 发现tslib是准的，但是Qt不准，应该是Qt的配置问题

在`/etc/profile`中有对Qt的配置
```
export  QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
export  QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=/dev/input/event1
```
发现是`QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS`设置的有问题，之前加了y轴的反转...

### 2.驱动模块如何开机时自动加载
- 将加载脚本放到`/etc/init.d/rcS`即可
```
insmod /lib/modules/4.1.15/touch_pannel.ko
insmod /lib/modules/4.1.15/LED_driver.ko
insmod /lib/modules/4.1.15/KEY_driver.ko
```

### 3.VSCODE里clangd插件跳转有问题
- 老是对没问题的代码报错, 比如std命名空间
- 代码跳转也有问题，比如跳转到头文件时，会跳转到宿主机根文件系统的头文件而不是交叉编译器的头文件

应该是编译器的参数设置的原因，之前正点原子给了个CMake交叉编译的模板，里面会显式设置交叉编译器，但我没有在`pro`文件里设置

- 解决方法：在`pro`文件里加上一些参数即可
```
# 编译器参数
TOOLCHAIN_DIR = /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf
QMAKE_CC  = $${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc
QMAKE_CXX = $${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++
QMAKE_LINK = $${QMAKE_CXX}

QMAKE_CFLAGS   += -march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7
QMAKE_CXXFLAGS += $${QMAKE_CFLAGS}
```

### 4.交叉编译器的组成
- 这个问题源于我想使用v4l2的时候，不知道头文件在哪，是该引入linux内核的头文件目录，还是说交叉编译器自带的头文件目录已经包含了v4l2的头文件？

一个典型的交叉编译工具链目录结构如下：
```
/usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/
├── bin/                     # 主机平台的可执行文件
│   ├── arm-linux-gnueabihf-gcc
│   ├── arm-linux-gnueabihf-g++
│   ├── arm-linux-gnueabihf-ld
│   └── ...
├── lib/                     # 主机平台的库文件
├── libexec/                 # 内部工具（如编译器后端）
├── share/                   # 共享数据（如文档、配置文件）
└── arm-linux-gnueabihf/     # 目标平台的工具链和库
    ├── bin/                 # 目标平台的可执行文件
    ├── include/             # 目标平台的头文件
    ├── lib/                 # 目标平台的库文件
    └── libc/                # 目标平台C标准库目录
        ├── usr/include/     # C标准库的头文件
        └── usr/lib/         # C标准库系统库文件
```
编译器有个重要的参数`sysroot`，它指定了目标平台的**​根文件系统**的路径。通过`sysroot`，交叉编译器可以找到目标平台所用**标准库**以及基本的Linux内核的头文件(用于与内核交互,使用内核提供的API比如V4L2)、库文件和其他资源，从而正确地编译和链接程序
- `sysroot`中的内核头文件的内核版本需要与目标机器的内核版本尽可能的一致，否则可能出现部分API、结构体的兼容性问题
- 交叉编译器的`sysroot`中会有内核版本的说明,在`/usr/include/linux/version.h`文件下中

### 5.include用<>还是""
搜索顺序：
- #include<>	
1. 编译器的`sysroot`路径（如 /usr/include）
2. -I 指定的路径

- #include""	
1. 当前文件所在目录
2. -I 指定的路径
3. 编译器的`sysroot`路径

### 6.代码运行是出现segment fault
segment fault是什么？
- 可以和page fault联系起来记忆，page fault是mmu访问的虚拟地址没有对应的物理内存而引发的异常，需要内核分配物理内存，而segment fault是程序访问的虚拟地址**不合法**而导致的程序崩溃，虚拟地址不合法的原因，大概率是数组越界之类的

解决思路:
- 出现这类问题，一定要利用gdb工具，它在程序崩溃的时候可以看到崩溃的位置以及栈帧，可以知道问题大概出在哪里，然后打印变量的值，分析问题到底出在哪里

### 7.i2c驱动和设备匹配不上
排查思路：
- 首先检查设备是否存在,使用`i2cdetect -y 1`命令检查硬件是否连接正确,如果地址显示UU则代表已经匹配上了
- 接着看一下`/sys/bus/i2c/`下的`device`和`driver`目录下,有没有对应的设备和驱动目录。如果有的话可以仅目录看一下有没有对应的驱动/设备，有的话就是已经匹配了
- 如果驱动和设备文件都有，但还是没匹配上，可能是代码出问题了。

驱动中与匹配相关的代码如下：
```c
static const struct of_device_id ap3216c_of_match[] = {
    {.compatible = "alientek,ap3216c"}, 
    {}
};
static struct i2c_device_id ap3216c_i2c_id[] = {
    {"ap3216c", 0}, 
    {}
};
static struct i2c_driver ap3216c_driver = {
    .driver =
        {
            .name = "ap3216c",                  // 名称匹配(用于板级代码静态注册时)
            .of_match_table = ap3216c_of_match, // 设备树匹配
        },
    .id_table = ap3216c_i2c_id, // id table匹配
    .probe = ap3216c_probe,
    .remove = ap3216c_remove,
};
```
注意：
- `i2c`总线匹配的方式跟`platform`差不多，都有好几种,但又不完全一样，具体定义在Linux源码中的`drivers/i2c/i2c-core.c`中。这里我遇到问题的主要原因是：`i2c`即使使用`of_match_table`来匹配，也**必须**要定义`id_table`，因为在完成匹配后进行`i2c_device_probe`时在要检测这个这个字段是否存在，如果不存在，即使匹配成功了也不会调用设备的`probe`
```c
struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,
	.probe		= i2c_device_probe,
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
};
```

### 8.开发板的Qt显示不了中文
- 问题原因：板子缺少中文字库
- 开发板的字库在`/usr/lib/fonts/`,自己下个中文字库文件放到这个路径里面就行了
硬件控制
## 参考链接
AP3216C:

- https://blog.csdn.net/mftang/article/details/136222319
- https://blog.csdn.net/zhengnianli/article/details/115222723