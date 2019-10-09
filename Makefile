
.PHONY: all

all:
	$(MAKE) -C src
	$(MAKE) -C libfas

clean:
	$(MAKE) -C src clean
	$(MAKE) -C libfas clean

code-format:
	./.custom-format.py -i src/*.c
	./.custom-format.py -i include/*.h
	./.custom-format.py -i libfas/*.c
	./.custom-format.py -i test/*.c
