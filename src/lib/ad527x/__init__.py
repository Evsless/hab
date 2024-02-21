from enum import Enum

__version__ = "1.0.0"

const = lambda x: x

class AD5272_REG(Enum):
    RDAC_R      = const(0x02)
    CFG_R       = const(0x08)
    CON_50TP_R  = const(0x05)
    LA_50TP_R   = const(0x06)