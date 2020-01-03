#KERNELDIR:= /home/linux/kernel/kernel-3.4.39/
KERNELDIR:= /lib/modules/$(shell uname -r)/build/
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
#	arm-none-linux-gnueabi-gcc test.c
#	cp *.ko  ~/rootfs/
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

obj-m:=atomic.o
