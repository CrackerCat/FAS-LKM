NAMES := test_open test_sigpipe test_multisessions
BINS := $(addsuffix .bin, $(NAMES))
OBJECTS := $(addsuffix .o, $(NAMES))

HDR_PATH ?= ../include/
LIB_PATH ?= ../libfas/

CFLAGS ?= -g -O0
CFLAGS += -I$(HDR_PATH) -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign

COMMON_HDR = $(HDR_PATH)fas.h

all: $(BINS)

%.bin: %.c $(COMMON_HDR)
	$(CC) $(CFLAGS) $< -o $@ $(LIB_PATH)/libfas.o $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) $(BINS)
