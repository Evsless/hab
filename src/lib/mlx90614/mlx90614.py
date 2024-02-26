import time
from i2clib.i2c_handler import i2c_handler

const = lambda x: x

# EEPROM REGISTERS
#   EEPROM_WRITE_CMD = (0x01 << 5) | EEPROM_ADDRESS

_MLX90614_EEPROM_T0_MAX     = const(0x20)
_MLX90614_EEPROM_T0_MIN     = const(0x21)
_MLX90614_EEPROM_PWMCTRL    = const(0x22)
_MLX90614_EEPROM_TA_RANGE   = const(0x23)
_MLX90614_EEPROM_EMISSIVITY = const(0x24)
_MLX90614_EEPROM_CFG_REG1   = const(0x25)
_MLX90614_EEPROM_SMBUS_ADDR = const(0x2E)

_MLX90614_EEPROM_ID_NUM1    = const(0x3C)
_MLX90614_EEPROM_ID_NUM2    = const(0x3D)
_MLX90614_EEPROM_ID_NUM3    = const(0x3E)
_MLX90614_EEPROM_ID_NUM4    = const(0x3F)


# RAM REGISTERS
#   RAM_WRITE_CMD = RAM_ADDRESS
_MLX90614_RAM_IR_RAW_CH1    = const(0x04)
_MLX90614_RAM_IR_RAW_CH1    = const(0x05)
_MLX90614_T_AMBIENT         = const(0x06)
_MLX90614_T_OBJ1            = const(0x06)
_MLX90614_T_OBJ2            = const(0x06)

# OTHER
_MLX90614_READ_FLAGS = const(0xF0)
_MLX90614_SLEEP_MODE = const(0xFF)


# UTILITIES
_TEMP_PRESCALER = 0.02
_TEMP_KELVIN    = 273.15

class mlx90614:
    def __init__(self, address, i2c_bus) -> None:
        self._address = address
        self._i2c     = i2c_bus
        self._r_buff  = bytearray(2)

    def _read_reg(self, reg) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))
            i2c_if.readfrom_into(self._address, self._r_buff)


    def _to_celcius(self, t_raw) -> float:
        t_celcius = t_raw * _TEMP_PRESCALER - _TEMP_KELVIN

        return t_celcius


    def read_ambient(self) -> float:
        self._read_reg(_MLX90614_T_AMBIENT)

        amb_raw = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)

        return self._to_celcius(amb_raw)


    def read_obj1(self) -> float:
        self._read_reg(_MLX90614_T_OBJ1)

        obj_raw = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)

        return self._to_celcius(obj1_raw)


    def read_obj2(self) -> float:
        self._read_reg(_MLX90614_T_OBJ2)

        obj_raw = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)

        return self._to_celcius(obj_raw)
