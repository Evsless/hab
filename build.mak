include build_support/kmod.mak
include build_support/makeutils.mak
include build_support/common_dirs.mak
include build_support/hrtim_trig.mak
include build_support/project_parts.mak

get_dev_idx 		= $(call decrement,$(call get_word_idx,$1,$(HAB_KMOD_LIST)))
define-iiodev-idx 	= -DIIO_KMOD_IDX_$(call to_upper,$1)=$(call get_dev_idx,$1)

PHONIES 			?=
HABMASTER_BIN_NAME 	:= hab_master
GPP_ARG_INCLUDE 	:= $(foreach header,$(HAB_INCLUDE_LIST),-I$(header))
GPP_ARG_PREPROC     := $(foreach habmod,$(HAB_KMOD_LIST),$(call define-iiodev-idx,$(habmod))) \
						-DIIO_TRIG_OFFSET=1


build_all_hab: $(HAB_SRC_LIST)
	@g++ -o hab_master $(HAB_SRC_LIST) $(GPP_ARG_INCLUDE) $(GPP_ARG_PREPROC)

PHONIES += test_print
test_print:
	@echo TEST_PRINT: $(PHONIES)

PHONY: $(PHONIES)