NAME = test_open.bin
OBJECTS = test_open.o

HDR_PATH ?= ../include/
LIB_PATH ?= ../libfas/

CFLAGS ?= -g -O0
CFLAGS += -I$(HDR_PATH) -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign

COMMON_HDR = $(HDR_PATH)fas.h

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(LIB_PATH)/libfas.o $(OBJECTS) $(LDFLAGS)

%.bin: %.c $(COMMON_HDR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(NAME)
