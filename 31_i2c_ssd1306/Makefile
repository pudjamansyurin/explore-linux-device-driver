TARGET = alman_oled
KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)

obj-m += $(TARGET).o 
$(TARGET)-objs += main.o ssd1306.o

all:
	make -C $(KDIR) M=$(CDIR) modules
clean:
	make -C $(KDIR) M=$(CDIR) clean
