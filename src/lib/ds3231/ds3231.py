from i2clib.i2c_handler import i2c_handler

const = lambda x: x


# STATUS/CONTROL REGISTERS
_DS3231_CONTROL_REG = const(0x0E)
_DS3231_STATUS_REG  = const(0x0F)

# READ REGISTERS
_DS3231_REG_SECONDS = const(0x00)
_DS3231_REG_MINUTES = const(0x01)
_DS3231_REG_HOURS   = const(0x02)
_DS3231_REG_DAY_W   = const(0x03)
_DS3231_REG_DAY_M   = const(0x04)
_DS3231_REG_CENT    = const(0x05)
_DS3231_REG_YEAR    = const(0x06)

_DS3231_REG_ALRM_1SEC   = const(0x07)
_DS3231_REG_ALRM_1MIN   = const(0x08)
_DS3231_REG_ALRM_1HRS   = const(0x09)
_DS3231_REG_ALRM_1DAY_W = const(0x0A)
_DS3231_REG_ALRM_1DAY_M = const(0x0B)

_DS3231_REG_ALRM1_SEC = const(0x07)
_DS3231_REG_ALRM1_MIN = const(0x08)
_DS3231_REG_ALRM1_HRS = const(0x09)
_DS3231_REG_ALRM1_DAY = const(0x0A)

_DS3231_REG_ALRM2_MIN = const(0x0B)
_DS3231_REG_ALRM2_HRS = const(0x0C)
_DS3231_REG_ALRM2_DAY = const(0x0D)

_DS3231_REG_TEMP_H  = const(0x11)
_DS3231_REG_TEMP_L  = const(0x12)

# MASKS
_MASK_CTRL_EOSC  = const(0x01 << 7)
_MASK_CTRL_BBSQW = const(0x01 << 6)
_MASK_CTRL_CONV  = const(0x01 << 5)
_MASK_CTRL_RS2   = const(0x01 << 4)
_MASK_CTRL_RS1   = const(0x01 << 3)
_MASK_CTRL_EN32  = const(0x01 << 3)
_MASK_CTRL_INTCN = const(0x01 << 2)
_MASK_CTRL_A2IE  = const(0x01 << 1)
_MASK_CTRL_A1IE  = const(0x00)

_MASK_STAT_OSF = const(0x01 << 7)
_MASK_STAT_BSY = const(0x01 << 2)
_MASK_STAT_A2F = const(0x01 << 1)
_MASK_STAT_A1F = const(0x00)

_MASK_TIME_PM_CFG = const(0x01 << 6)


class ds3231:
    def __init__(self, address, i2c_bus):
        self._address = address
        self._i2c     = i2c_bus


    def _read_reg(self, reg, buff) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))
            i2c_if.readfrom_into(self._address, buff)


    def _write_reg(self, reg, val) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg, val]))

    # TBD: COVER THE OSCILATER ENABLING SCENARIOS
    def _osc_en(self):
        pass


    def _is_busy(self) -> bool:
        buff = bytearray(1)
        self._read_reg(_DS3231_STATUS_REG, buff)

        return buff[0] & _MASK_STAT_BSY

    def _read_seconds(self) -> int:
        r_buff = bytearray(1)
        self._read_reg(_DS3231_REG_SECONDS, r_buff)
        
        seconds_10 = (r_buff[0] >> 4) & 0x0F
        seconds    = r_buff[0] & 0x0F

        return seconds_10 * 10 + seconds


    def _read_minutes(self) -> int:
        r_buff = bytearray(1)
        self._read_reg(_DS3231_REG_MINUTES, r_buff)

        minutes_10 = (r_buff[0] >> 4) & 0x0F
        minutes    = r_buff[0] & 0x0F

        return minutes_10 * 10 + minutes


    def _read_hours(self) -> int:
        r_buff = bytearray(1)
        self._read_reg(_DS3231_REG_HOURS, r_buff)

        hrs    = r_buff[0] & 0x0F

        if r_buff[0] & _MASK_TIME_PM_CFG:
            # Set some PM flag?
            hrs_10 = (r_buff[0] >> 4) & 0x01
        else:
            hrs_10 = (r_buff[0] >> 4) & 0x03

        return hrs_10 * 10 + hrs


    def set_pm_mode(self) -> None:
        r_buff = bytearray(1)

        self._read_reg(_DS3231_REG_HOURS, r_buff)
        self._write_reg(_DS3231_REG_HOURS, r_buff[0] | _MASK_TIME_PM_CFG)


    def set_20hrs_mode(self) -> None:
        r_buff = bytearray(1)

        self._read_reg(_DS3231_REG_HOURS, r_buff)
        self._write_reg(_DS3231_REG_HOURS, (~_MASK_TIME_PM_CFG & 0xFF) & r_buff[0])


    def update_temp(self) -> None:
        r_buff = bytearray(1)
        # 1. Read current configuration
        self._read_reg(_DS3231_CONTROL_REG, r_buff)

        # 2. Wait untill any operation finishes
        while self._is_busy():
            pass

        # 3. Init a temperature read & TCXO calibration
        self._write_reg(_DS3231_CONTROL_REG, _MASK_CTRL_CONV)

        # 4. Wait untill conversion finishes
        self._read_reg(_DS3231_CONTROL_REG, r_buff)
        while r_buff[0] & _MASK_CTRL_CONV:
            self._read_reg(_DS3231_CONTROL_REG, r_buff)

    def read_temp(self) -> float:
        r_buff = bytearray(2)

        self._read_reg(_DS3231_REG_TEMP_H, r_buff)

        temp = r_buff[0] + (r_buff[1] >> 6) * 0.25
    
        return temp

    def set_time(self, hr, min, sec) -> None:
        buff = bytearray(1)
        self._read_reg(_DS3231_REG_HOURS, buff)

        hrs_cfg = buff[0] & const(0x03 << 6)

        if hr >= 0 and hr <= 23:
            l_nibble = hr % 10
            h_nibble = hr // 10
            self._write_reg(_DS3231_REG_HOURS, (hrs_cfg | (h_nibble << 4)) | l_nibble)
        
        if min >= 0 and min <= 59:
            self._write_reg(_DS3231_REG_MINUTES, ((min // 10) << 4) | (min % 10))

        if sec >= 0 and sec <= 59:
            self._write_reg(_DS3231_REG_SECONDS, ((sec // 10) << 4) | (sec % 10))


    def read_time(self) -> tuple:
        return (self._read_hours(), \
                self._read_minutes(), \
                self._read_seconds())
