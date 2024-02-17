

from .utils import *

class ads1115:
    def __init__(self, address, i2c) -> None:
        self._address = address
        self._i2c     = i2c
        self._r_buff  = bytearray(3)

    def write(self, reg, val):
        while not self._i2c.try_lock():
            pass

        self._i2c.writeto(self._address, bytearray([reg, (val >> 8) & 0xFF, val & 0x0F]))

    def read(self, reg):
        while not self._i2c.try_lock():
            pass

        self._i2c.writeto(self._address, bytearray([reg, 0x00, 0x00]))
        self._i2c.readfrom_into(self._address, self._r_buff)