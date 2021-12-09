obj-m += device_file.o 

KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)

all:
	make -C $(KDIR) M=$(CDIR) modules

clean:
	make -C $(KDIR) M=$(CDIR) clean
