obj-m += attiny-i2c-pwm-bl.o

DEPMOD=$(shell which depmod)
SRC=$(shell pwd)

.PHONY: clean

all:
	@echo make: "$(MAKE)"
	@echo src: "$(SRC)"
	@echo depmod: "$(DEPMOD)"
	make -C /lib/modules/$(shell uname -r)/build M=$(SRC) modules

# most likely requires sudo permissions to run this
modules_install:
	@echo make: "$(MAKE)"
	@echo src: "$(SRC)"
	@echo depmod: "$(DEPMOD)"

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(SRC) modules_install
	$(DEPMOD)

clean:
	rm -f $(wildcard *.mod) $(wildcard *.mod.*) $(wildcard *.o) modules.order Module.symvers $(wildcard *.ko) $(wildcard .*.o) $(wildcard .*.o.d) $(wildcard .*.cmd)
