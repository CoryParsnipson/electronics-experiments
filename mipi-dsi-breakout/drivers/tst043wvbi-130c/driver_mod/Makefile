obj-m += panel-ilitek-ili9806e-dsi.o

DEPMOD=$(shell which depmod)
SRC=$(shell pwd)


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
