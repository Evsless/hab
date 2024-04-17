########################################################################################################################
# KERNEL MODULES LIST
########################################################################################################################
KMOD_MPRLS0025 := mprls0025
KMOD_ICM20948  := icm20x
KMOD_SHT40     := sht4x
KMOD_ADS1115   := ads1115
KMOD_AD5272    := ad5272
KMOD_MLX90614  := mlx90614

HAB_KMOD_LIST += $(KMOD_MPRLS0025) 		\
					$(KMOD_ICM20948) 	\
					$(KMOD_SHT40) 		\
					$(KMOD_ADS1115) 	\
					$(KMOD_AD5272) 		\
					$(KMOD_MLX90614)

########################################################################################################################
# DEVICE LIST
########################################################################################################################
HABDEV_MPRLS := mprls0025
HABDEV_ICM20X := icm20x
HABDEV_SHT40 := sht4x
HABDEV_ADS1115_48 := ads1115_48
HABDEV_ADS1115_49 := ads1115_49
HABDEV_AD5272_2C := ad5272_2c
HABDEV_AD5272_2E := ad5272_2e
HABDEV_AD5272_2F := ad5272_2f
HABDEV_MLX90614 := mlx90614

HABDEV_LIST := $(HABDEV_MPRLS) \
				$(HABDEV_ICM20X) \
				$(HABDEV_SHT40) \
				$(HABDEV_ADS1115_48) \
				$(HABDEV_ADS1115_49) \
				$(HABDEV_AD5272_2C) \
				$(HABDEV_AD5272_2E) \
				$(HABDEV_AD5272_2F) \
				$(HABDEV_MLX90614)

HABDEV_CALLBACK_EV := $(HABDEV_MPRLS)%tim_ev \
						$(HABDEV_ICM20X)%fs_ev \
						$(HABDEV_SHT40)%tim_ev

########################################################################################################################
# TRIGGER LIST
########################################################################################################################
TRIG_5000  := _5000
TRIG_20000 := _20000
TRIG_0     := _0

########################################################################################################################
# DEVICE-TRIGGER LUT
########################################################################################################################
$(HABDEV_MPRLS)_TRIG := $(TRIG_5000)
$(HABDEV_ICM20X)_TRIG := $(TRIG_0)
$(HABDEV_SHT40)_TRIG := $(TRIG_20000)

rem_rep = $(strip $(shell echo $1 | tr ' ' '\n' | sort -u | tr '\n' ' '))
_TRIG_LIST = $(foreach elem,$(HABDEV_LIST),$($(elem)_TRIG))
TRIG_LIST = $(call rem_rep,$(_TRIG_LIST))

########################################################################################################################
# MACRO DEFINITIONS
########################################################################################################################
HABDEV_IDX_ARRAY 	= $(call create_array,$(call indexify,$(HABDEV_LIST)))
TRIG_ARRAY 			= $(call create_array,$(subst _,$(EMPTY),$(TRIG_LIST)))

_TRIG_LUT_RAW 		= $(foreach elem,$(HABDEV_LIST),$(call get_arr_idx,$($(elem)_TRIG),$(TRIG_LIST)))
TRIG_LUT_ARRAY 		= $(call create_array,$(_TRIG_LUT_RAW))

HABDEV_MACRO_LIST = $(foreach habmod,$(HABDEV_LIST),$(call define-dev-macro-name,$(habmod)))

# CALLBACKS
HABDEV_CB_NAME_LIST = $(foreach habdev,$(HABDEV_LIST),$(call define-dev-callback-macro,$(habdev)))

get-ev-type = $(lastword $(subst %,$(SPACE),$1))
get-dev     = $(firstword $(subst %,$(SPACE),$1))
_TIMER_EV_DEV_IDX = $(foreach callback,$(HABDEV_CALLBACK_EV),\
						$(if $(call str-eq,$(call get-ev-type,$(callback)),tim_ev),\
							$(call get_arr_idx,$(call get-dev,$(callback)),$(HABDEV_LIST))-,\
							$(EMPTY)\
						)\
					)


TIMER_EV_DEV_IDX = {\
	$(foreach idx,$(_TIMER_EV_DEV_IDX),\
		$(if $(call str-eq,$(idx),$(lastword $(_TIMER_EV_DEV_IDX))),\
			$(subst -,$(EMPTY),$(idx)),\
			$(subst -,$(COMMA),$(idx))\
		)\
	)\
}
