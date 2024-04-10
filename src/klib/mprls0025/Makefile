MODULE_NAME 		:= mprls0025
KMOD_SETUP_LIST 	:= industrialio \
						industrialio-triggered-buffer \
						iio-trig-hrtimer
	
DT_OVERLAYS_LIST 	:= $(MODULE_NAME).dtbo

IIO_DEVICE_SYSFS_NAME := /sys/bus/iio/devices/iio\:device0
DEVICE_CHMOD_LIST := $(IIO_DEVICE_SYSFS_NAME)/trigger/current_trigger \
						$(IIO_DEVICE_SYSFS_NAME)/buffer/length \
						$(IIO_DEVICE_SYSFS_NAME)/buffer/enable \
						$(IIO_DEVICE_SYSFS_NAME)/scan_elements/in_pressure_en \
						$(IIO_DEVICE_SYSFS_NAME)/scan_elements/in_timestamp_en \

obj-m += $(MODULE_NAME).o

all: kmod mprls0025.dtbo
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

setup_trig:
	@sudo mount -t configfs none /config
	@sudo mkdir /config/iio/triggers/hrtimer/trig_2s
	@sudo chmod 666 /sys/bus/iio/devices/trigger0/sampling_frequency
	@echo  0.5 > /sys/bus/iio/devices/trigger0/sampling_frequency

setup_buffer:
	$(foreach iio_sysfs,$(DEVICE_CHMOD_LIST),sudo chmod 666 $(iio_sysfs);)

	cat /sys/bus/iio/devices/trigger0/name > $(IIO_DEVICE_SYSFS_NAME)/trigger/current_trigger
	@echo 1 > $(IIO_DEVICE_SYSFS_NAME)/scan_elements/in_pressure_en
	@echo 1 > $(IIO_DEVICE_SYSFS_NAME)/scan_elements/in_timestamp_en
	@echo 4 > $(IIO_DEVICE_SYSFS_NAME)/buffer/length
	@echo 1 > $(IIO_DEVICE_SYSFS_NAME)/buffer/enable

PHONIES += dtoverlay_load setup_buffer

.PHONY: $(PHONIES)