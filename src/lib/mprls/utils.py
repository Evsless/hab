from . import const


MPRLS_CONST_ADDR_DEFAULT = const(0x18)

MPRLS_COMMAND_READ_DATA  = const(0xAA)

MPRLS_MASK_BUSY                = const(0x20)
MPRLS_MASK_MATH_SATURATION_ERR = const(0x01)
MPRLS_MASK_INTEGRITY_ERR       = const(0x02)

MPRLS_PRESCALER_PA       = const(float(6894.76))
MPRLS_PRESCALER_HPA      = const(float(68.9476))

MPRSL_CAL_RESOLUTION      = const(2 ** 24)
MPRLS_CAL_MIN_OP_PRESSURE = const(float(0.1))
MPRLS_CAL_MAX_OP_PRESSURE = const(float(0.9))