QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ui/hardware_widget.cpp \
    ui/map_widget.cpp \
    ui/menu_widget.cpp \
    ui/music_widget.cpp \
    ui/video_widget.cpp \
    hardware/LED.cpp \

HEADERS += \
    mainwindow.h \
    ui/hardware_widget.h \
    ui/map_widget.h \
    ui/menu_widget.h \
    ui/music_widget.h \
    ui/video_widget.h
    ui/common.h

INCLUDEPATH += \
    /home/lrq/linux/IMX6ULL/tool/plog/include/ \
    /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/include/ \
    hardware/ \

FORMS += \
    mainwindow.ui \
    ui/hardware_widget.ui \
    ui/map_widget.ui \
    ui/menu_widget.ui \
    ui/music_widget.ui \
    ui/video_widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
