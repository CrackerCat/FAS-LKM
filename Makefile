
.PHONY: all

all:
	cd src && $(MAKE)
	$(MAKE) -C libfas

clean:
	cd src && $(MAKE) clean
	$(MAKE) -C libfas clean

code-format:
	./.custom-format.py -i src/*.c
	./.custom-format.py -i include/*.h
	./.custom-format.py -i libfas/*.c
	./.custom-format.py -i test/*.c
