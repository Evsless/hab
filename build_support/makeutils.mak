########################################################################################################################
# UTILITY FUNCTIONS																									   #
########################################################################################################################

# FUNCTION: to_upper
#
# DESCRIPTION:
#     Upper case for a given string
# ARGS:
#     $1 - a string to do uppercase on
to_upper   = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')

# FUNCTION: decrement
#
# DESCRIPTION:
#     Decrement given index by 1
# ARGS:
#     $1 - a number that must be decremented
decrement  = $(shell echo "$$(($(1) - 1))")

# FUNCTION: get_word_idx
#
# DESCRIPTION:
#     Gen an argument of a given string from the list.
#     An index starts with 1.
# ARGS:
#     $1 - A string for which index must be found;
#     $2 - Set of strings
# EXAMPLE:
#     INPUT:
#         $(call get_word_idx,arm,x86 risc-v arm)
#     OUTPUT:
#         3
_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       	$(wordlist 2,$(words $2),$2),x $3),$3)
get_word_idx = $(words $(call _pos,$1,$2))

# FUNCTION: get_arr_idx
#
# DESCRIPTION:
#     The function does the same as get_word_idx, but now
#     an index starts with 0.
# ARGS:
#     $1 - A string for which index must be found;
#     $2 - Set of strings
# EXAMPLE:
#     INPUT:
#         $(call get_word_idx,x86,x86 risc-v arm)
#     OUTPUT:
#         0
get_arr_idx = $(call decrement,$(call get_word_idx,$1,$2))

# FUNCTION: indexify
#
# DESCRIPTION:
#     The function gets a string, then returns a string of indexes of its elements.
# ARGS:
#     $1 - string whose elements will be indexed
# EXAMPLE:
#     INPUT:
#         val_1 val_2 val3
#     OUTPUT:
#         0 1 2
indexify = $(foreach elem,$1,$(call get_arr_idx,$(elem),$1))

# FUNCTION: create_array
#
# DESCRIPTION:
#      Creates a C-compatible array from the given
#      string of space separated elements.
# ARGS:
#      $1 - a string with space separated elements
# EXAMPLE:
#     INPUT:
#         0 5000 10000
#     OUTPUT:
#         {0, 5000, 10000} $(elem)$(COMMA)
create_array = {$(foreach elem,$1,$(if $(call str-eq,$(elem),$(EMPTY)),$(EMPTY),$(if $(call str-eq,$(elem),$(lastword $1)),$(elem),$(elem)$(COMMA))))}

# FUNCTION: to_string
#
# DESCRIPTION:
#      Creates a string from the given element. Might be used for passing
#      a string macro to the compiler
# ARGS:
#      $1 - an elements that must be casted to string
to_string = '"$1"'

# FUNCTION: str-eq
#
# DESCRIPTION:
#      Check if given strings are equal. If not, return empty, otherwise the given string.
# ARGS:
#      $1 - first string string to be compared;
#      $2 - second string string to be compared.
str-eq = $(strip $(filter $1,$2))

# FUNCTION: define-dev-callback-name
#
# DESCRIPTION:
#      Defines a callback name that will be used inside the application code.
# ARGS:
#      $1 - callback basename (device code);
define-dev-callback-name = '$1_callback'

# FUNCTION: define-dev-macro-name
#
# DESCRIPTION:
#      Defines a macro name with device ID value, that later will be used inside an app.
# ARGS:
#      $1 - device name stored inside $(HABDEV_LIST);
define-dev-macro-name = -DIIO_KMOD_IDX_$(call to_upper,$1)=$(call get_arr_idx,$1,$(HABDEV_LIST))

# FUNCTION: remove_repetition
#
# DESCRIPTION:
#      Removes repetitions in the given string.
# ARGS:
#      $1 - a string where repetitions must be removed.
remove_repetition = $(strip $(shell echo $1 | tr ' ' '\n' | sort -u | tr '\n' ' '))

########################################################################################################################
# UTILITY VARIABLES																									   #
########################################################################################################################

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
COMMA :=,

TIM_CB := tim
FS_CB  := fs