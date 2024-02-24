from .      import const
from .utils import *
from i2clib.i2c_handler import i2c_handler

import time

class icm20948:
    def __init__(self, address, i2c_bus) -> None:
        self._address   = const(address)
        self._i2c       = i2c_bus
        self._fifo_buff = bytearray(4096)

        self._init()


    def _init(self):

        self.reset()

        # 0. Setup the UB0
        self._set_bank(MASK_REG_BANK_SEL_UB_0)

        # 1. PWR_MGMT: Wake up & disable temp
        self._write_reg(ICM20948_REG_PWR_MGMT_1, MASK_PWR_MGMT_TEMP_DIS | MASK_PWR_MGMT_CLKSEL)

        # 2. USER_CTRL: Enable the DMP & FIFO
        self._write_reg(ICM20948_REG_USER_CTRL, MASK_USER_CTRL_FIFO_EN)

        # 3. PWR_MGMT_2: Disable the gyroscope
        self._write_reg(ICM20948_REG_PWR_MGMT_2, MASK_PWR_MGMT_2_DISABLE_GYRO)

        # 4. INT_ENABLE_2: Enable the FIFO overflow interrupt
        self._write_reg(ICM20948_REG_INT_ENABLE_2, MASK_INT_ENABLE_2_FIFO_OVERFLOW_EN)

        # 5. FIFO_EN_2: Setup accel write to FIFO
        self._write_reg(ICM20948_REG_FIFO_EN_2, MASK_FIFO_EN_2_ACCEL_FIFO_EN)

        # 6. FIFO_MODE: Setup a stream mode for FIFO
        self._write_reg(ICM20948_REG_FIFO_MODE, MASK_FIFO_MODE_SNAPSHOT)

        self._set_bank(MASK_REG_BANK_SEL_UB_2)

        # 8. Setup a sample rate divider
        #    Default config SR: 1125 / (1 + 10) = 102 Hz
        self._write_reg(ICM20948_REG_ACCEL_SMPLRT_DIV_2, 0x0A)

        # 9. Setup an alignment of ODR when changing the SR
        self._write_reg(ICM20948_REG_ODR_ALIGN_EN, MASK_ODR_ALIGN_EN_ON)

        # 10. ACCEL_CONFIG: Setup an FSR for +- 2g
        self._write_reg(ICM20948_REG_ACCEL_CONFIG, \
                MASK_ACCEL_CONFIG_ACCEL_FS_SEL_2G | MASK_ACCEL_CONFIG_ACCEL_FCHOICE_ON)

        # 11. Reset the FIFO
        self.reset_fifo()

    
    def _set_bank(self, ubank) -> None:
        with i2c_handler(self._i2c) as i2c_if: 
            i2c_if.writeto(self._address, bytearray([ICM20948_REG_REG_BANK_SEL, ubank]))

    def _write_reg(self, reg, val = 0x00) -> None:
        with i2c_handler(self._i2c) as i2c_if: 
            i2c_if.writeto(self._address, bytearray([reg, val]))

    def _read_reg(self, reg, buff) -> None:
        with i2c_handler(self._i2c) as i2c_if:
            i2c_if.writeto(self._address, bytearray([reg]))
            i2c_if.readfrom_into(self._address, buff)

    def _to_int(self, val) -> int:
        if val > 0x7FFF:
            val -= (0x01 << 16)

        return val

    def reset(self) -> None:
        self._set_bank(MASK_REG_BANK_SEL_UB_0)
        self._write_reg(ICM20948_REG_PWR_MGMT_1, 0x01 << 7)
        time.sleep(0.01)

    def sleep(self, pwr_status:bool) -> None:
        self._set_bank(MASK_REG_BANK_SEL_UB_0)

        self._read_reg()

    def reset_fifo(self):
        self._set_bank(MASK_REG_BANK_SEL_UB_0)
        self._write_reg(ICM20948_REG_FIFO_RST, 0x01)
        self._write_reg(ICM20948_REG_FIFO_RST, 0x00)

    def read_fifo(self) -> bytearray():
        self._set_bank(MASK_REG_BANK_SEL_UB_0)

        # 1. Read the FIFO len
        buffer = bytearray(2)
        self._read_reg(ICM20948_REG_FIFO_COUNTH, buffer)
        fifo_len = (buffer[0] << 8) | (buffer[1] & 0xFF)

        # 2. Read the FIFO
        self._read_reg(ICM20948_REG_R_W, self._fifo_buff)
    
        # 3. FIFO reset
        self.reset_fifo()
    
        return (self._fifo_buff, fifo_len)

    def read_accel(self):
        acc_buff = bytearray(6)
        self._set_bank(MASK_REG_BANK_SEL_UB_0)

        self._read_reg(ICM20948_REG_ACCEL_XOUT_H, acc_buff)

        xa_raw = self._to_int((acc_buff[0] << 8) | (acc_buff[1] & 0xFF))
        ya_raw = self._to_int((acc_buff[2] << 8) | (acc_buff[3] & 0xFF))
        za_raw = self._to_int((acc_buff[4] << 8) | (acc_buff[5] & 0xFF))

        return xa_raw, ya_raw, za_raw


    def read(self, bank, reg):
        buff = bytearray(1)
        self._set_bank(bank)
        self._read_reg(reg, buff)

        return buff[0]
