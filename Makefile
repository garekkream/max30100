obj-m := max30100.o

KDIR ?= ../linux/

all:
	make ARCH=arm -C $(KDIR) M=$(PWD) modules

clean:
	make ARCH=arm -C $(KDIR) M=$(PWD) clean
