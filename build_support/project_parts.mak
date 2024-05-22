########################################################################################################################
# LIST OF HEADER FILES																								   #
########################################################################################################################
# 1. CORE PLATFORM HEADERS
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/hab
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/event
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/common
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/stdtypes
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/hab_trig
HAB_INCLUDE_LIST 	+= $(HAB_CORE_INC_PATH)/iio_buffer_ops

# 2. GENERATED DATA HEADERS
HAB_INCLUDE_LIST	+= $(HAB_OUT_GENERATED_PATH)

# 3. USER APPLICATION HEADERS
HAB_INCLUDE_LIST 	+= $(HAB_USR_INC_PATH)/

########################################################################################################################
# LIST OF SOURCE FILES																								   #
########################################################################################################################
# 1. CORE PLATFORM SRC
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/main.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/hab/hab.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/event/event.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/event/callback.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/dfa.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/utils.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/llist.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/cfg_tree.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/hab_device.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/hab_trig/hab_trig.c
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/iio_buffer_ops/iio_buffer_ops.c

# 2. USER APPLICATION SRC
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/camera.c
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/task_main.c
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/wheatstone.c
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/ff_detector.c
