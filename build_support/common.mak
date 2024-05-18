########################################################################################################################
# KERNEL MODULES LIST
########################################################################################################################
KMOD_MPRLS0025 := mprls0025
KMOD_ICM20948  := icm20x
KMOD_SHT40     := sht4x
KMOD_ADS1115   := ads1115
KMOD_AD5272    := ad5272
KMOD_MLX90614  := mlx90614

HAB_KMOD_LIST := $(KMOD_MPRLS0025) 		\
					$(KMOD_ICM20948) 	\
					$(KMOD_SHT40) 		\
					$(KMOD_ADS1115) 	\
					$(KMOD_AD5272) 		\
					$(KMOD_MLX90614)

MDT_KMOD_LED := hab_led
MDT_KMOD_LIST := $(MDT_KMOD_LED)

########################################################################################################################
# DEVICE LIST
########################################################################################################################
HABDEV_MPRLS := mprls0025
HABDEV_ICM20948 := icm20948
HABDEV_SHT40 := sht4x
HABDEV_ADS1115_48 := ads1115_48
HABDEV_ADS1115_49 := ads1115_49
HABDEV_AD5272_2C := ad5272_2c
HABDEV_AD5272_2E := ad5272_2e
HABDEV_AD5272_2F := ad5272_2f
HABDEV_MLX90614 := mlx90614
HABDEV_CAM_1 := imx477_01
HABDEV_CAM_2 := imx477_02

HABDEV_LIST := $(HABDEV_MPRLS) \
				$(HABDEV_ICM20948) \
				$(HABDEV_SHT40) \
				$(HABDEV_ADS1115_48) \
				$(HABDEV_ADS1115_49) \
				$(HABDEV_AD5272_2C) \
				$(HABDEV_AD5272_2E) \
				$(HABDEV_AD5272_2F) \
				$(HABDEV_MLX90614) \
				$(HABDEV_CAM_1) \
				$(HABDEV_CAM_2)

########################################################################################################################
# DEVICE-EVENT HASHTABLE
########################################################################################################################
$(HABDEV_MPRLS)_EV 			:= $(TIM_CB)
# $(HABDEV_ICM20948)_EV 		:= $(TIM_CB)
$(HABDEV_SHT40)_EV 			:= $(TIM_CB)
$(HABDEV_ADS1115_48)_EV 	:= $(TIM_CB)
$(HABDEV_ADS1115_49)_EV 	:= $(TIM_CB)
$(HABDEV_MLX90614)_EV 		:= $(TIM_CB)
$(HABDEV_CAM_1)_EV 			:= $(TIM_CB)
$(HABDEV_CAM_2)_EV 			:= $(TIM_CB)

########################################################################################################################
# TRIGGER LIST
########################################################################################################################
TRIG_500   := _500
TRIG_1000  := _1000
TRIG_5000  := _5000
TRIG_20000 := _20000
TRIG_10000 := _10000
TRIG_0     := _0

########################################################################################################################
# DEVICE-TRIGGER HASHTABLE
########################################################################################################################
$(HABDEV_MPRLS)_TRIG 		:= $(TRIG_20000)
$(HABDEV_ICM20948)_TRIG 	:= $(TRIG_500)
$(HABDEV_SHT40)_TRIG 		:= $(TRIG_20000)
$(HABDEV_ADS1115_48)_TRIG 	:= $(TRIG_1000)
$(HABDEV_ADS1115_49)_TRIG 	:= $(TRIG_5000)
$(HABDEV_MLX90614)_TRIG 	:= $(TRIG_20000)

_TRIG_LIST = $(foreach elem,$(HABDEV_LIST),$($(elem)_TRIG))
TRIG_LIST = $(call remove_repetition,$(_TRIG_LIST))

########################################################################################################################
# MACRO DEFINITIONS
########################################################################################################################
# Device indexes. Used for identifying a device inside the application
HABDEV_IDX_ARRAY 	= $(call create_array,$(call indexify,$(HABDEV_LIST)))
_DEV_NAMES = $(foreach dev,$(HABDEV_LIST),\"$(dev)\")
DEV_NAMES = $(call create_array,$(_DEV_NAMES))

# Trigger delay values. Used when registering trigger.
TRIG_ARRAY 			= $(call create_array,$(subst _,$(EMPTY),$(TRIG_LIST)))

# Trigger lookup table, the table is alligned with HABDEV_IDX_ARRAY. Elements inside are TRIG_ARRAY indexes.
_TRIG_LUT_RAW 		= $(foreach elem,$(HABDEV_LIST),$(call get_arr_idx,$($(elem)_TRIG),$(TRIG_LIST)))
TRIG_LUT_ARRAY 		= $(call create_array,$(_TRIG_LUT_RAW))

# Device indexes of timer triggered events
_TIMER_EV_DEV_IDX = $(foreach elem,$(HABDEV_LIST),$(if $(call str-eq,$($(elem)_EV),$(TIM_CB)),$(call get_arr_idx,$(elem),$(HABDEV_LIST)),$(EMPTY)))
TIMER_EV_DEV_IDX  = $(call create_array,$(_TIMER_EV_DEV_IDX))

HABDEV_MACRO_LIST = $(foreach habmod,$(HABDEV_LIST),$(call define-dev-macro-name,$(habmod)))

# Callback names. Used when implementing callback for a device.
_USED_CALLBACKS = $(foreach habdev,$(HABDEV_LIST),$(if $($(habdev)_EV),$(habdev),$(EMPTY)))
HABDEV_CB_NAME_LIST = $(foreach cb,$(_USED_CALLBACKS),-D$(call to_upper,$(cb))_CALLBACK=$(call define-dev-callback-name,$(cb)))

# Callback list - a list of callbacks from HABDEV_CB_NAME_LIST, that must be used within the application.
_CB_LIST = $(foreach cb,$(_USED_CALLBACKS),$(call define-dev-callback-name,$(cb)))
CB_LIST = $(call create_array,$(_CB_LIST))
