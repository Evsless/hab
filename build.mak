include build_support/kmod.mak
include build_support/common.mak
include build_support/makeutils.mak
include build_support/common_dirs.mak
include build_support/hrtim_trig.mak
include build_support/project_parts.mak

PHONIES 			?=
HABMASTER_BIN_NAME 	:= $(HAB_OUT_BIN_PATH)/hab_master
GPP_ARG_INCLUDE 	:= $(foreach header,$(HAB_INCLUDE_LIST),-I$(header))
GPP_ARG_PREPROC     := $(HABDEV_MACRO_LIST) \
						-DHAB_DEV_NAME='$(DEV_NAMES)' \
						$(HABDEV_CB_NAME_LIST) \
						-DHAB_CALLBACKS='$(CB_LIST)' \
						-DHABDEV_IDX_SET='$(HABDEV_IDX_ARRAY)' \
						-DTRIG_PERIOD_SET='$(TRIG_ARRAY)' \
						-DTRIG_LUT='$(TRIG_LUT_ARRAY)' \
						-DEV_TIM_DEV_IDX='$(TIMER_EV_DEV_IDX)' \
						-DHAB_BUFF_CFG_PATH=$(call to_string,$(HAB_BUFF_CFG_PATH)/)


build_all_hab: $(HABMASTER_BIN_NAME)
$(HABMASTER_BIN_NAME): $(HAB_SRC_LIST)
	@mkdir -p $(dir $(HABMASTER_BIN_NAME))
	@g++ -o $(HABMASTER_BIN_NAME) $(HAB_SRC_LIST) $(GPP_ARG_INCLUDE) $(GPP_ARG_PREPROC) -luv -g
PHONIES += build_all_hab

PHONIES += test_print
test_print:
	@echo HABDEV_IDX_ARRAY: $(HABDEV_MACRO_LIST)
	@echo TRIG_ARRAY: 	  	$(TRIG_ARRAY)
	@echo TRIG_LUT_ARRAY:   $(TRIG_LUT_ARRAY)
	@echo TIMER_EV_DEV_IDX: $(TIMER_EV_DEV_IDX)
	@echo HABDEV_CB_NAME_LIST: $(HABDEV_CB_NAME_LIST)
	@echo CB_LIST: $(CB_LIST)
	@echo DEVICE_NAME: $(DEV_NAMES)


PHONY: $(PHONIES)