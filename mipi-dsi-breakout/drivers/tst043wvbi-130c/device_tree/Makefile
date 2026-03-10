all:
	dtc -I dts -O dtb tst043wvbi-overlay.dts -o tst043wvbi-overlay.dtbo
	mv tst043wvbi-overlay.dtbo /boot/overlays/

dump:
	dtc -I fs -O dts -o device-tree-dump.txt /sys/firmware/devicetree/base
