from . import const

class ad5272_controller:
    def __init__(self, address, i2c_bus) -> None:
        self._address = const(address)
        self._i2c     = i2c_bus
        self._r_buff  = bytearray(2)

    def write(self, reg, val) -> None:
        while not self._i2c.try_lock():
            pass

        try:
            self._i2c.writeto(self._address, bytearray([(reg << 2) | ((val >> 8) & 0x03), val & 0xFF]))
        finally:
            self._i2c.unlock()

    def read(self, reg) -> int:
        while not self._i2c.try_lock():
            pass

        try:
            self._i2c.writeto(self._address, bytearray([reg << 2, 0x00]))
            self._i2c.readfrom_into(self._address, self._r_buff)
            result = ((self._r_buff[0] & 0x03) << 8) | self._r_buff[1]

        finally:
            self._i2c.unlock()

        return result
