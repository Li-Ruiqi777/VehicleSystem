# VehicleSystem
基于嵌入式Linux的智能车载系统

## TODO
- [ ] 引入PLOG库
- [ ] 开发HardWareControl部分

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