MODULE_NAME 	:= ad5272
KMOD_SETUP_LIST := industrialio
DT_OVERLAYS_LIST := $(MODULE_NAME).dtbo

IIO_SYSFS_DEVICE_BASE := /sys/bus/iio/devices/iio

IIO_SYSFS_DEVICE_LIST := $(IIO_SYSFS_DEVICE_BASE)\:device0 \
							$(IIO_SYSFS_DEVICE_BASE)\:device1 \
							$(IIO_SYSFS_DEVICE_BASE)\:device2

obj-m += $(MODULE_NAME).o

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

reload_module: clean all
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

module_enable:
	@sudo insmod $(MODULE_NAME).ko
	$(foreach dev,$(IIO_SYSFS_DEVICE_LIST),sudo chmod 666 $(dev)/out_resistance_raw;)

PHONIES += dtoverlay_load reload_module module_enable
 
.PHONY: $(PHONIES)