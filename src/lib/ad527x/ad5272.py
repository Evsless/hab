from .      import const
from .utils import *
from i2clib.i2c_handler import i2c_handler

class ad5272:
    def __init__(self, address, i2c_bus) -> None:
        self._address = const(address)
        self._i2c     = i2c_bus
        self._r_buff  = bytearray(2)

        self.write_cfg(AD5272_CFG_WP_DIS)

    def _write_reg(self, reg, val = 0x00) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([(reg << 2) | ((val >> 8) & 0x03), val & 0xFF]))

    def _read_response(self) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.readfrom_into(self._address, self._r_buff)

    def write_rdac(self, val) -> None:
        if AD5272_CONST_RDAC_MIN <= val <= AD5272_CONST_RDAC_MAX:
            self._write_reg(AD5272_CMD_WRITE_RDAC, val)
        else:
            pass

    def write_cfg(self, val) -> None:
        if 0 <= val <= 0x0F:
            self._write_reg(AD5272_CMD_WRITE_CTRL_REGISTER, val)
        else:
            pass

    def read(self, reg) -> int:
        if reg in (AD5272_CMD_READ_RDAC, \
                    AD5272_CMD_READ_CTRL_REGISTER, \
                    AD5272_CMD_READ_50TP_CONTENT, \
                    AD5272_CMD_READ_50TP_LAST_ADDR):
            self._write_reg(reg)
            self._read_response()
        else:
            pass

        return ((self._r_buff[0] & 0x03) << 8) | self._r_buff[1]
