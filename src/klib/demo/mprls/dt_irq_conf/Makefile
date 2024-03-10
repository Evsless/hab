obj-m += mprls0025.o

all: kmod mprls0025.dtbo
	@echo "Kernel module & device tree overlay build.\n"

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

mprls0025.dtbo: mprls0025.dts
	dtc -@ -I dts -O dtb -o $@ mprls0025.dts

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

dtoverlay_load:
	@sudo modprobe industrialio
	@sudo dtoverlay mprls0025.dtbo

.PHONY: dtoverlay_load