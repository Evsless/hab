from enum import Enum

const = lambda x: x

class ADS1115_CH(Enum):
    CH0 = const(0x00)
    CH1 = const(0x01)
    CH2 = const(0x02)
    CH3 = const(0x03)
    CH01 = const(0x00 << 12)
    CH23 = const(0x03 << 12)