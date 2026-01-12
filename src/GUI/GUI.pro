QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 编译器参数
TOOLCHAIN_DIR = /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf
QMAKE_CC  = $${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc
QMAKE_CXX = $${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++
QMAKE_LINK = $${QMAKE_CXX}

QMAKE_CFLAGS   += -march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7 \
                  -Wno-missing-field-initializers -Wno-unused-variable -Wno-sign-compare \
                  -g -O3
QMAKE_CXXFLAGS += $${QMAKE_CFLAGS}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ui/hardware_widget.cpp \
    ui/map_widget.cpp \
    ui/menu_widget.cpp \
    ui/music_widget.cpp \
    ui/video_widget.cpp \
    ui/backupview_widget.cpp \
    hardware/LED.cpp \
    hardware/V4L2Camera.cpp \
    hardware/AP3216C.cpp

HEADERS += \
    mainwindow.h \
    ui/hardware_widget.h \
    ui/map_widget.h \
    ui/menu_widget.h \
    ui/music_widget.h \
    ui/video_widget.h \
    ui/backupview_widget.h \
    ui/common.h

INCLUDEPATH += \
    /home/lrq/linux/ThirdPart/plog/include \
    hardware \

FORMS += \
    mainwindow.ui \
    ui/hardware_widget.ui \
    ui/map_widget.ui \
    ui/menu_widget.ui \
    ui/music_widget.ui \
    ui/video_widget.ui \
    ui/backupview_widget.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
