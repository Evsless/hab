########################################################################################################################
# INCLUDES																											   #
########################################################################################################################
include build_support/common.mak
include build_support/common_dirs.mak

########################################################################################################################
# HAB DEVICES																										   #
########################################################################################################################

IIO_KMOD_LIST := industrialio \
					industrialio-triggered-buffer \
					regmap-i2c \
					crc8

HAB_KMOD_BASENAME_LIST := $(foreach kmod,$(HAB_KMOD_LIST),$(HAB_KLIB_PATH)/$(kmod)/$(kmod))
MDT_KMOD_BASENAME_LIST := $(foreach kmod,$(MDT_KMOD_LIST),$(HAB_KLIB_PATH)/$(kmod)/$(kmod))

########################################################################################################################
# KERNEL MODULES && DEVICE TREE OVERLAY BUILD TARGETS																   #
########################################################################################################################
.PHONIES += build_mdt_kmod
build_mdt_kmod: $(addsuffix .ko,$(MDT_KMOD_BASENAME_LIST))
$(addsuffix .ko,$(MDT_KMOD_BASENAME_LIST)): $(addsuffix .c,$(MDT_KMOD_BASENAME_LIST))
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(dir $@) modules
	@mkdir -p $(HAB_OUT_KLIB_PATH)
	@ln -f $@ $(HAB_OUT_KLIB_PATH)

.PHONIES += build_kmod
build_kmod: $(addsuffix .ko,$(HAB_KMOD_BASENAME_LIST))
$(addsuffix .ko,$(HAB_KMOD_BASENAME_LIST)): $(addsuffix .c,$(HAB_KMOD_BASENAME_LIST))
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(dir $@) modules
	@mkdir -p $(HAB_OUT_KLIB_PATH)
	@ln -f $@ $(HAB_OUT_KLIB_PATH)

.PHONIES += build_dtoverlay
build_dtoverlay: $(addsuffix .dtbo,$(HAB_KMOD_BASENAME_LIST))
$(addsuffix .dtbo,$(HAB_KMOD_BASENAME_LIST)): $(addsuffix .dts,$(HAB_KMOD_BASENAME_LIST))
	@dtc -@ -I dts -O dtb -o $@ $(basename $@).dts
	@mkdir -p $(HAB_OUT_DTOVERLAY_PATH)
	@echo "Creating symbolic link for $@ in $(HAB_OUT_DTOVERLAY_PATH)"
	@ln -f $@ $(HAB_OUT_DTOVERLAY_PATH)


.PHONIES += clean_all
clean_all:
	@$(foreach kdir,$(HAB_KMOD_BASENAME_LIST) $(MDT_KMOD_BASENAME_LIST),\
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/$(dir $(kdir)) clean;)
	@$(foreach kdir,$(HAB_KMOD_BASENAME_LIST) $(MDT_KMOD_BASENAME_LIST),sudo rm -f $(kdir).dtbo)
	@sudo rm -rf $(HAB_OUT_KLIB_PATH) $(HAB_OUT_DTOVERLAY_PATH)


########################################################################################################################
# LOADERS FOR KERNEL MODULES && DTOVERLAYS																			   #
########################################################################################################################
.PHONIES += load_dtoverlay
load_dtoverlay: $(addsuffix .dtbo,$(HAB_KMOD_BASENAME_LIST))
	@echo "----------------------------------------------"	
	@$(foreach iio_kmod,$(IIO_KMOD_LIST),\
		echo "INFO: loading iio module $(iio_kmod)." && sudo modprobe $(iio_kmod);)
	@echo "----------------------------------------------"
	@$(foreach overlay,$(HAB_KMOD_LIST),\
		echo "INFO: loading dtoverlay $(overlay).dtbo" && sudo dtoverlay $(HAB_OUT_DTOVERLAY_PATH)/$(overlay).dtbo;)
	@echo "----------------------------------------------"

.PHONIES += load_kmod
load_mdt_kmod: $(addsuffix .ko,$(MDT_KMOD_BASENAME_LIST))
	@echo "----------------------------------------------"
	@$(foreach kmod,$(MDT_KMOD_LIST),\
		echo "INFO: loading kernel module $(kmod).ko" && sudo insmod $(HAB_OUT_KLIB_PATH)/$(kmod).ko;)
	@echo "----------------------------------------------"

.PHONIES += load_kmod
load_kmod: $(addsuffix .ko,$(HAB_KMOD_BASENAME_LIST))
	@echo "----------------------------------------------"
	@$(foreach kmod,$(HAB_KMOD_LIST),\
		echo "INFO: loading kernel module $(kmod).ko" && sudo insmod $(HAB_OUT_KLIB_PATH)/$(kmod).ko;)
	@echo "----------------------------------------------"

.PHONIES += setup_kernel_all
setup_kernel_all: build_dtoverlay load_dtoverlay build_kmod load_kmod build_mdt_kmod load_mdt_kmod
	@echo "INFO: Kernel setup finished. Modules loaded: $(HAB_KMOD_LIST) $(MDT_KMOD_LIST)."
