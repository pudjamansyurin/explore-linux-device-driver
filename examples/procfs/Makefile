TARGET = alm_procfs
KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)

obj-m += $(TARGET).o 
$(TARGET)-objs += procfs4.o

all:
	make -C $(KDIR) M=$(CDIR) modules
clean:
	make -C $(KDIR) M=$(CDIR) clean
