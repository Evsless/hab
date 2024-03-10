obj-m += mprls0025.o
KMOD_SETUP_LIST := industrialio industrialio-triggered-buffer
DT_OVERLAYS_LIST := mprls0025.dtbo

all: kmod mprls0025.dtbo
	@echo "Kernel module & device tree overlay build.\n"

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

mprls0025.dtbo: mprls0025.dts
	dtc -@ -I dts -O dtb -o $@ mprls0025.dts

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

dtoverlay_load:
	$(foreach mod,$(KMOD_SETUP_LIST),sudo modprobe $(mod);)
	$(foreach dtbo,$(DT_OVERLAYS_LIST),sudo dtoverlay $(dtbo);)

PHONIES += dtoverlay_load

.PHONY: $(PHONIES)