import time

from . import const
from i2clib.i2c_handler import i2c_handler

_SHT40_HPRES_NO_HEAT = const(0xFD)
_SHT40_MPRES_NO_HEAT = const(0xF6)
_SHT40_LPRES_NO_HEAT = const(0xE0)
_SHT40_SERIAL_NUM    = const(0x89)
_SHT40_SOFT_RESET    = const(0x94)

_SHT40_HPRES_HEAT_200_1s    = const(0x39)
_SHT40_HPRES_HEAT_200_100ms = const(0x32)
_SHT40_HPRES_HEAT_110_1s    = const(0x2F)
_SHT40_HPRES_HEAT_110_100ms = const(0x24)
_SHT40_HPRES_HEAT_20_1s     = const(0x1E)
_SHT40_HPRES_HEAT_20_100ms  = const(0x15)

_DELAY_HREP = 0.0083
_DELAY_MREP = 0.0045
_DELAY_LREP = 0.0045

_HEATING_1s    = 1.1
_HEATING_100ms = 0.11

_POWERUP_TIME  = 0.001

class sht40:
    def __init__(self, address, i2c_bus) -> None:
        self._address   = const(address)
        self._i2c       = i2c_bus
        self._r_buff    = bytearray(6)
        self._dtime     = _DELAY_HREP

    def _write_reg(self, reg):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))

    def _read_reg(self, reg):
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))
        
            time.sleep(self._dtime)

            i2c_if.readfrom_into(self._address, self._r_buff)

    def _convert_temp(self, temp_raw) -> float:
        temp = (-45 + 175 * temp_raw / (2 ** 16 - 1))
        return temp

    def _convert_humid(self, humid_raw) -> float:
        humid = (-6 + 125 * humid_raw / (2 ** 16 - 1))
        return humid

    def _get_processed_data(self) -> tuple:
        # 1. Extract the raw values
        temp_raw  = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)
        humid_raw = (self._r_buff[3] << 8) | (self._r_buff[4] & 0xFF)
        
        # 2. Convert raw reads to physical values
        temp  = self._convert_temp(temp_raw)
        humid = self._convert_humid(humid_raw)

        return (temp, humid)

    def reset(self):
        self._write_reg(_SHT40_SOFT_RESET)
        time.sleep(_POWERUP_TIME)

    def read_serial_num(self):
        self._read_reg(_SHT40_SERIAL_NUM)

        snum_word1 = (self._r_buff[0] << 8) | (self._r_buff[1] & 0xFF)
        snum_word2 = (self._r_buff[3] << 8) | (self._r_buff[4] & 0xFF)

        return (snum_word1, snum_word2)

    def read_hpres(self):
        # 1. Set the delay time
        self._dtime = _DELAY_HREP

        # 2. Read the raw data
        self._read_reg(_SHT40_HPRES_NO_HEAT)

        # 3. Process the data to physical values and return
        return self._get_processed_data()

    def read_mpres(self):
        self._dtime = _DELAY_MREP

        self._read_reg(_SHT40_MPRES_NO_HEAT)

        return self._get_processed_data()

    def read_lpres(self):
        self._dtime = _DELAY_LREP

        self._read_reg(_SHT40_LPRES_NO_HEAT)

        return self._get_processed_data()

    def read_heat_200_1s(self):
        self._dtime = _DELAY_HREP + _HEATING_1s

        self._read_reg(_SHT40_HPRES_HEAT_200_1s)

        return self._get_processed_data()

    def read_heat_200_100ms(self):
        self._dtime = _DELAY_HREP + _HEATING_100ms

        self._read_reg(_SHT40_HPRES_HEAT_200_100ms)

        return self._get_processed_data()

    def read_heat_110_1s(self):
        self._dtime = _DELAY_HREP + _HEATING_1s

        self._read_reg(_SHT40_HPRES_HEAT_110_1s)

        return self._get_processed_data()

    def read_heat_110_100ms(self):
        self._dtime = _DELAY_HREP + _HEATING_100ms

        self._read_reg(_SHT40_HPRES_HEAT_110_100ms)

        return self._get_processed_data()

    def read_heat_20_1s(self):
        self._dtime = _DELAY_HREP + _HEATING_1s

        self._read_reg(_SHT40_HPRES_HEAT_20_1s)

        return self._get_processed_data()

    def read_heat_20_100ms(self):
        self._dtime = _DELAY_HREP + _HEATING_100ms

        self._read_reg(_SHT40_HPRES_HEAT_20_100ms)

        return self._get_processed_data()