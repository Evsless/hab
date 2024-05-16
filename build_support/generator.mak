include build_support/common_dirs.mak

GEN_PATH_EV_MAIN := $(HAB_OUT_GENERATED_PATH)/ev_main.h
GEN_PATH_EV_GLOB := $(HAB_OUT_GENERATED_PATH)/ev_glob.h

GEN_PATH_ALL = $(GEN_PATH_EV_MAIN) \
				$(GEN_PATH_EV_GLOB)

gen_main_ev: $(GEN_PATH_EV_MAIN)
$(GEN_PATH_EV_MAIN):
	@mkdir -p $(dir $@)
	@echo "*INFO: Generating $@."
	@printf '%s\n' \
		"#ifndef __EV_MAIN_H__"\
		"#define __EV_MAIN_H__"\
		""\
		"#define EV_MAIN_HAB_EXT \"hab_evmain\""\
		"#define EV_MAIN_CALLBACK ev_main_callback"\
		""\
		"#define EV_GLOB_LIST {EV_MAIN_HAB_EXT}"\
		""\
		"#endif /* __EV_MAIN_H__ */" > $@


gen_glob_ev: $(GEN_PATH_EV_GLOB)
$(GEN_PATH_EV_GLOB):
	@mkdir -p $(dir $@)
	@echo "*INFO: Generating $@."
	@printf '%s\n' \
		"#ifndef __EV_GLOB_H__"\
		"#define __EV_GLOB_H__"\
		""\
		"#include \"ev_main.h\""\
		""\
		"#define EV_GLOBAL_CB_LIST {EV_MAIN_CALLBACK}"\
		""\
		"#endif /* __EV_GLOB_H__ */" > $@

gen_all: gen_main_ev gen_glob_ev
	@echo "*INFO: Generated all the required data."
