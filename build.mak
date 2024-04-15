include build_support/kmod.mak
include build_support/common.mak
include build_support/makeutils.mak
include build_support/common_dirs.mak
include build_support/hrtim_trig.mak
include build_support/project_parts.mak

get_dev_idx 		= $(call decrement,$(call get_word_idx,$1,$(HAB_KMOD_LIST)))
define-iiodev-idx 	= -DIIO_KMOD_IDX_$(call to_upper,$1)=$(call get_dev_idx,$1)

PHONIES 			?=
HABMASTER_BIN_NAME 	:= $(HAB_OUT_BIN_PATH)/hab_master
GPP_ARG_INCLUDE 	:= $(foreach header,$(HAB_INCLUDE_LIST),-I$(header))
GPP_ARG_PREPROC     := $(foreach habmod,$(HAB_KMOD_LIST),$(call define-iiodev-idx,$(habmod))) \
						-DIIO_TRIG_OFFSET=1 \
						-DHAB_BUFF_CFG_PATH=$(call to_string,$(HAB_BUFF_CFG_PATH)/)


build_all_hab: $(HABMASTER_BIN_NAME)
$(HABMASTER_BIN_NAME): $(HAB_SRC_LIST)
	@mkdir -p $(dir $(HABMASTER_BIN_NAME))
	@g++ -o $(HABMASTER_BIN_NAME) $(HAB_SRC_LIST) $(GPP_ARG_INCLUDE) $(GPP_ARG_PREPROC)
PHONIES += build_all_hab

PHONIES += test_print
test_print:
	@echo TEST_PRINT: $(pwd)

PHONY: $(PHONIES)