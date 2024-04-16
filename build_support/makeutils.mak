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
#         {0, 5000, 10000}
create_array = {$(subst $(SPACE),$(COMMA) ,$1)}

# FUNCTION: to_string
#
# DESCRIPTION:
#      Creates a string from the given element. Might be used for passing
#      a string macro to the compiler
# ARGS:
#      $1 - an elements that must be casted to string
to_string = '"$1"'

########################################################################################################################
# UTILITY VARIABLES																									   #
########################################################################################################################

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
COMMA :=,