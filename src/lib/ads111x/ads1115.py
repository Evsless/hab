

from .utils import *
from i2clib.i2c_handler import i2c_handler

class ads1115:
    def __init__(self, address, i2c) -> None:
        self._address = address
        self._i2c     = i2c
        self._r_buff  = bytearray(2)
        self._config  = ADS1115_CONFIG_DEFAULT

    def _write_reg(self, reg, val):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg, (val >> 8) & 0xFF, val & 0xFF]))

    def _read_active_reg(self):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.readfrom_into(self._address, self._r_buff)

    def _set_active_reg(self, reg):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))

    def _reset(self):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(I2C_GEN_CALL_ADDR, bytearray([ADS1115_OP_RESET]))

    def _conv_ready(self):
        self._read_active_reg()
        reg = self._r_buff[0] << 8 | self._r_buff[1]
        return reg & ADS1115_MASK_CFG_OP_STATUS

    def _to_int(self, val):
        if val > 0x7FFF:
            val -= (0x01 << 16)

        return val

    def read(self, channel):
        self._config &= ADS1115_MASK_CLEAR_CHANNEL
        self._write_reg(ADS1115_ADDR_CONF_REG,
                        self._config | ADS1115_MASK_CFG_OP_STATUS | channel << 12)
        
        while not self._conv_ready():
            pass

        self._set_active_reg(ADS1115_ADDR_CONV_REG)
        self._read_active_reg()

        vol_raw = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)
        vol_raw = self._to_int(vol_raw)

        return vol_raw