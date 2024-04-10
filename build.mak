include build_support/common_dirs.mak
include build_support/project_parts.mak

PHONIES 			?= test
HABMASTER_BIN_NAME 	:= hab_master
GPP_ARG_INCLUDE 	:= $(foreach header,$(HAB_INCLUDE_LIST),-I$(header))

build_all_hab: $(HAB_SRC_LIST)
	@g++ -o hab_master $(HAB_SRC_LIST) $(GPP_ARG_INCLUDE)

test:
	@echo g++ $(HAB_SRC_LIST) $(GPP_ARG_INCLUDE) -o $(HABMASTER_BIN_NAME)

PHONY: $(PHONIES)