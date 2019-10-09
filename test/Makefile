NAME = test_open.bin
OBJECTS = test_open.o

HDR_PATH ?= ../include/
LIB_PATH ?= ../libfas/

CFLAGS ?= -g -O0
CFLAGS += -I$(HDR_PATH) -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign
LDFLAGS += -L$(LIB_PATH) -lfas

COMMON_HDR = $(HDR_PATH)fas.h

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

%.bin: %.c $(COMMON_HDR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(NAME)
