BR2_EXTERNAL_KERNEL_MODULE_PATH ?= $(PWD)

obj-m += $(addsuffix .o, $(notdir $(basename $(wildcard $(BR2_EXTERNAL_KERNEL_MODULE_PATH)/*.c))))
ccflags-y := -DDEBUG -g -I$(src)/../include

LINUX_DIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C '$(LINUX_DIR)' 'M=$(PWD)' modules

clean:
	make -C '$(LINUX_DIR)' 'M=$(PWD)' clean
