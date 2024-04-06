obj-m += icm20x.o
KMOD_SETUP_LIST := industrialio industrialio-triggered-buffer
DT_OVERLAYS_LIST := icm20x.dtbo
# ccflags-y := -I.

DEVICE_0 := /sys/bus/iio/devices/iio\:device0
IIO_BUFFER_LEN := 2048

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
	sudo rmmod icm20x
	sudo insmod icm20x.ko


setup_buffer:
	sudo chmod 666 $(DEVICE_0)/trigger/current_trigger
	sudo chmod 666 $(DEVICE_0)/scan_elements/in_accel_x_en
	sudo chmod 666 $(DEVICE_0)/scan_elements/in_accel_y_en
	sudo chmod 666 $(DEVICE_0)/scan_elements/in_accel_z_en
	sudo chmod 666 $(DEVICE_0)/buffer/length
	sudo chmod 666 $(DEVICE_0)/buffer/enable

	cat /sys/bus/iio/devices/trigger0/name > $(DEVICE_0)/trigger/current_trigger
	@echo 1 > $(DEVICE_0)/scan_elements/in_accel_x_en
	@echo 1 > $(DEVICE_0)/scan_elements/in_accel_y_en
	@echo 1 > $(DEVICE_0)/scan_elements/in_accel_z_en
	@echo $(IIO_BUFFER_LEN) > $(DEVICE_0)/buffer/length
	@echo 1 > $(DEVICE_0)/buffer/enable

buffer_disable:
	@echo 0 > $(DEVICE_0)/buffer/enable
	@echo > $(DEVICE_0)/trigger/current_trigger

PHONIES += dtoverlay_load reload_module setup_buffer

.PHONY: $(PHONIES)