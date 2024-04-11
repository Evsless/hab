########################################################################################################################
# UTILITY FUNCTIONS																									   #
########################################################################################################################

# FUNCTION: get_word_idx
# DESCRIPTION:
#     Gen an argument of a given string from the list
# ARGS:
#     $1 - A string for which index must be found;
#     $2 - Set of strings
_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       	$(wordlist 2,$(words $2),$2),x $3),$3)
get_word_idx = $(words $(call _pos,$1,$2))

# FUNCTION: to_upper
# DESCRIPTION:
#     Upper case for a given string
# ARGS:
#     $1 - a string to do uppercase on
to_upper   = $(shell echo '$1' | tr '[:lower:]' '[:upper:]')

# FUNCTION: decrement
# DESCRIPTION:
#     Decrement given index by 1
# ARGS:
#     $1 - a number that must be decremented
decrement  = $(shell echo "$$(($(1) - 1))")

########################################################################################################################
# UTILITY VARIABLES																									   #
########################################################################################################################

EMPTY = ''