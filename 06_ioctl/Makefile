obj-m += alman.o 

KDIR = /lib/modules/$(shell uname -r)/build
CDIR = $(shell pwd)
APP = "app"

all:
	make -C $(KDIR) M=$(CDIR) modules
	gcc -Wall -o $(APP) "$(APP).c"
clean:
	make -C $(KDIR) M=$(CDIR) clean
	rm $(APP)
