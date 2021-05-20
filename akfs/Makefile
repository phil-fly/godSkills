#engine Makefile
KDIR ?= /usr/src/kernels/`uname -r`
all:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
