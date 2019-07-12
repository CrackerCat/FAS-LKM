NAME = libfas

HDR_PATH ?= ../include/

CFLAGS ?= -g -O0
CFLAGS += -I$(HDR_PATH) -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign

COMMON_HDR = $(HDR_PATH)fas.h

SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $@.so $(OBJECTS) $(LDFLAGS)

%.o: %.c $(COMMON_HDR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(NAME).so
