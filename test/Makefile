NAMES := test_open test_sigpipe test_multisessions test_fuzz
SO_NAMES := test_preloader
BINS := $(addsuffix .bin, $(NAMES))
SO_BINS := $(addsuffix .so, $(SO_NAMES))
OBJECTS := $(addsuffix .o, $(NAMES)) $(addsuffix .o, $(SO_NAMES))

HDR_PATH ?= ../include/
LIB_PATH ?= ../libfas/

CFLAGS ?= -g -O0
CFLAGS += -I$(HDR_PATH) -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign
SO_CFLAGS := $(CFLAGS) -fPIC -shared

LDFLAGS ?= -pthread
SO_LDFLAGS ?= -ldl

COMMON_HDR = $(HDR_PATH)fas.h

all: $(BINS) $(SO_BINS)

%.bin: %.c $(COMMON_HDR)
	$(CC) $(CFLAGS) $< -o $@ $(LIB_PATH)/libfas.o $(LDFLAGS)

%.so: %.c $(COMMON_HDR)
	$(CC) $(SO_CFLAGS) $< -o $@ $(LIB_PATH)/libfas.o $(SO_LDFLAGS)


clean:
	rm -rf $(OBJECTS) $(BINS) $(SO_BINS)
