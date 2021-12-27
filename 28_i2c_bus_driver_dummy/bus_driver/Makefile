obj-m += alman_bus.o 

KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)

all:
	make -C $(KDIR) M=$(CDIR) modules
clean:
	make -C $(KDIR) M=$(CDIR) clean
