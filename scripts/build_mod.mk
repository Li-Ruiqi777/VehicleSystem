# 内核路径
KERNELDIR := /home/lrq/linux/IMX6ULL/temp
# 当前路径
CURRENT_PATH := $(shell pwd)
# 模块安装路径
MODULE_INSTALL_PATH := /home/lrq/linux/nfs/qtrootfs/lib/modules/4.1.15

# GCC 环境脚本
GCC_ENV := . /home/lrq/enable_gcc4.sh

# 默认目标
.PHONY: all install kernel_modules clean clean_modules
all: install

# 安装模块
install: kernel_modules
	@echo "Installing kernel modules to $(MODULE_INSTALL_PATH)"
	mkdir -p -m 755 $(MODULE_INSTALL_PATH)
	cp -f *.ko $(MODULE_INSTALL_PATH)/
ifeq ($(FIRMWARE_CLEAN),1)
	$(MAKE) clean
endif

# 编译内核模块
kernel_modules:
	@echo "Building kernel modules..."
	$(GCC_ENV) && $(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

# 清理模块生成文件
clean:
	@echo "Cleaning build artifacts..."
	$(GCC_ENV) && $(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean

# 删除已安装的模块
clean_modules:
	@echo "Removing installed modules from $(MODULE_INSTALL_PATH)"
	rm -f $(MODULE_INSTALL_PATH)/*.ko
