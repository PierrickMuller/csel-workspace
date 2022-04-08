export PATH := ${PATH}:/buildroot/output/host/usr/sbin/:/buildroot/output/host/usr/bin/:/buildroot/output/host/sbin/:/buildroot/output/host/bin/

ifneq ($(KERNELRELEASE),)
obj-m += mydriver3.o
mydriver3-objs := exo3.o
CFLAGS_skeleton := -DDEBUG

else
# Can be include in a differnet file -> add Modpath as well (/rootfs)
CPU := arm64
KDIR := /buildroot/output/build/linux-5.15.21/
TOOLS := /buildroot/output/host/usr/bin/aarch64-buildroot-linux-gnu-
PWD := $(shell pwd)
MODPATH := /rootfs

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules
install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif