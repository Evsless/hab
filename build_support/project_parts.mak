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

# 2. USER APPLICATION HEADERS
HAB_INCLUDE_LIST 	+= $(HAB_USR_INC_PATH)/

########################################################################################################################
# LIST OF SOURCE FILES																								   #
########################################################################################################################
# 1. CORE PLATFORM SRC
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/main.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/hab/hab.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/event/event.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/event/callback.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/utils.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/parser.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/common/hab_device.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/hab_trig/hab_trig.cpp
HAB_SRC_LIST += $(HAB_CORE_SRC_PATH)/iio_buffer_ops/iio_buffer_ops.cpp

# 2. USER APPLICATION SRC
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/wheatstone.cpp
HAB_SRC_LIST += $(HAB_USR_SRC_PATH)/ff_detector.cpp