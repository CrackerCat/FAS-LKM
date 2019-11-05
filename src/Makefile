ifndef M
override M = $(PWD)
endif

obj-m += fas.o
fas-objs := main.o open.o utils.o close.o sysfs.o

ccflags-y := \
  -DDEBUG \
  -ggdb3 \
  -std=gnu99 \
  -Wframe-larger-than=1000000000 \
  -I$(M)/../include \
  -Wno-declaration-after-statement \
  $(CCFLAGS)

LINUX_DIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all

all:
	$(MAKE) -C '$(LINUX_DIR)' M='$(M)' modules

clean:
	$(MAKE) -C '$(LINUX_DIR)' M='$(M)' clean
