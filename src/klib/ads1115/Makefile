obj-m += ads1115.o
KMOD_SETUP_LIST := industrialio industrialio-triggered-buffer
DT_OVERLAYS_LIST := ads1115.dtbo

all: kmod ads1115.dtbo
	@echo "Kernel module & device tree overlay build.\n"

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ads1115.dtbo: ads1115.dts
	dtc -@ -I dts -O dtb -o $@ $<

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

dtoverlay_load:
	$(foreach mod,$(KMOD_SETUP_LIST),sudo modprobe $(mod);)
	$(foreach dtbo,$(DT_OVERLAYS_LIST),sudo dtoverlay $(dtbo);)

PHONIES += dtoverlay_load

.PHONY: $(PHONIES)