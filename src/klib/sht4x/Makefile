obj-m += sht4x.o
KMOD_SETUP_LIST := industrialio industrialio-triggered-buffer
DT_OVERLAYS_LIST := sht4x.dtbo

all: kmod $(DT_OVERLAYS_LIST)
	@echo "Kernel module & device tree overlay build.\n"

kmod:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

$(DT_OVERLAYS_LIST): $(basename $(DT_OVERLAYS_LIST)).dts
	dtc -@ -I dts -O dtb -o $@ $<

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

dtoverlay_load:
	$(foreach mod,$(KMOD_SETUP_LIST),sudo modprobe $(mod);)
	$(foreach dtbo,$(DT_OVERLAYS_LIST),sudo dtoverlay $(dtbo);)

PHONIES += dtoverlay_load

.PHONY: $(PHONIES)