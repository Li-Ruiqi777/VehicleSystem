KERNELDIR := /home/lrq/linux/IMX6ULL/temp
CURRENT_PATH := $(shell pwd)
MODULE_INSTALL_PATH := /home/lrq/linux/nfs/qtrootfs/lib/modules/4.1.15

build: kernel_modules
	mkdir -m 755 -p ${MODULE_INSTALL_PATH}/
	mv *.ko ${MODULE_INSTALL_PATH}/
ifeq ($(FIRMWARE_CLEAN),1)
	make clean
endif

kernel_modules:
	$(MAKE) -s -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -s -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	
clean_modulues:
	rm $(MODULE_INSTALL_PATH)/*.ko
