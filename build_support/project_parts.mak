########################################################################################################################
# LIST OF HEADER FILES																								   #
########################################################################################################################
HAB_INCLUDE_LIST 	+= $(HAB_INLCUDE_PATH)/hab
HAB_INCLUDE_LIST 	+= $(HAB_INLCUDE_PATH)/common
HAB_INCLUDE_LIST 	+= $(HAB_INLCUDE_PATH)/stdtypes
HAB_INCLUDE_LIST 	+= $(HAB_INLCUDE_PATH)/hab_trig
HAB_INCLUDE_LIST 	+= $(HAB_INLCUDE_PATH)/iio_buffer_ops

########################################################################################################################
# LIST OF SOURCE FILES																								   #
########################################################################################################################
HAB_SRC_LIST += $(HAB_SRC_PATH)/main.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/hab/hab.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/common/utils.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/common/event.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/common/parser.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/common/hab_device.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/hab_trig/hab_trig.cpp
HAB_SRC_LIST += $(HAB_SRC_PATH)/iio_buffer_ops/iio_buffer_ops.cpp