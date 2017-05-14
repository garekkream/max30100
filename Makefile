MODUL=max30100
obj-m := max30100.o
max30100-objs := max30100_sysfs.o

KDIR ?= ../linux/

all:
	make ARCH=arm -C $(KDIR) M=$(PWD) modules

clean:
	make ARCH=arm -C $(KDIR) M=$(PWD) clean
