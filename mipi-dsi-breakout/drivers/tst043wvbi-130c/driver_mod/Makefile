DEPMOD=$(shell which depmod)
KERNEL_SRC=/usr/src/linux-headers-$(shell uname -r)
SRC=$(shell pwd)


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

modules_install:
	@echo make: "$(MAKE)"
	@echo kernel_src: "$(KERNEL_SRC)"
	@echo src: "$(SRC)"
	@echo depmod: "$(DEPMOD)"

	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install
	$(DEPMOD)
