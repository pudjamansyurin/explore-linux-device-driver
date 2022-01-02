TARGET = alman_bus

KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)

obj-m += $(TARGET).o 
$(TARGET)-objs += i2c_bus.o i2c_gpio.o

all:
	make -C $(KDIR) M=$(CDIR) modules
clean:
	make -C $(KDIR) M=$(CDIR) clean
