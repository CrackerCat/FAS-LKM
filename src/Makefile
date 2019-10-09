ifndef M
override M = $(PWD)
endif

obj-m += fas.o
fas-objs := main.o ioctl.o open.o utils.o release.o flush.o initial_path.o

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
