from . import const
from .utils import *

class MprlsDriver():
    def __init__(self, addr, i2c_bus, psi_min = 0, psi_max = 25) -> None:
        self._address  = addr
        self._i2c     = i2c_bus
        self._r_buff  = bytearray(4)

        self._pmin    = psi_min
        self._pmax    = psi_max

    def _is_busy(self) -> bool:
        self._i2c.readfrom_into(self._address, self._r_buff)
        return self._r_buff[0] & MPRLS_MASK_BUSY

    def _convert_data(self):
        data_raw = (self._r_buff[1] << 16) | (self._r_buff[2] << 8) | self._r_buff[3]

        raw_min_count = round(MPRLS_CAL_MIN_OP_PRESSURE * MPRSL_CAL_RESOLUTION)
        raw_max_count = round(MPRLS_CAL_MAX_OP_PRESSURE * MPRSL_CAL_RESOLUTION)

        data_psi = ((data_raw - raw_min_count) * (self._pmax - self._pmin)) / \
                    (raw_max_count - raw_min_count) + \
                    self._pmin

        return data_psi

    def read(self):
        s_buff = bytearray([0xAA, 0x00, 0x00])
        while not self._i2c.try_lock():
            pass

        try:
            self._i2c.writeto(self._address, s_buff)

            while self._is_busy():
                pass
            self._i2c.readfrom_into(self._address, self._r_buff)

            # TBD: DIAGNOSTICS SERVER?
            if self._r_buff[0] & MPRLS_MASK_MATH_SATURATION_ERR:
                pass
            elif self._r_buff[0] & MPRLS_MASK_INTEGRITY_ERR:
                pass
            else:
                pass

            data_psi = self._convert_data()

        finally:
            self._i2c.unlock()

        return data_psi * MPRLS_PRESCALER_PA

