include build_support/common.mak

IIO_TRIG_HRTIM_MOD  := iio-trig-hrtimer
IIO_TRIG_HRTIM_PATH := /config/iio/triggers/hrtimer
CONFIGFS_DT_PATH    := /config/device-tree

configfs_mount: $(CONFIGFS_DT_PATH)
$(CONFIGFS_DT_PATH):
	@echo "INFO: mounting configfs to /config."
	@sudo mount -t configfs none /config
PHONIES += configfs_mount

hrtim_trig_enable: $(IIO_TRIG_HRTIM_PATH)
$(IIO_TRIG_HRTIM_PATH): $(CONFIGFS_DT_PATH)
	@echo "INFO: Loading $(IIO_TRIG_HRTIM_MOD) module."
	@sudo modprobe $(IIO_TRIG_HRTIM_MOD)
