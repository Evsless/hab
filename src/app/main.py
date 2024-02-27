import board
import busio
import time

from ads111x.ads1115 import ads1115
from ads111x import ADS1115_CH

from ad527x.ad5272 import ad5272
from ad527x import AD5272_REG

from icm20x.icm20948 import icm20948
from icm20x.utils    import *

from sht4x.sht40 import sht40

from ds3231.ds3231 import ds3231

i2c0 = busio.I2C(board.D1, board.D0)
i2c1 = busio.I2C(board.D3, board.D2)

ads1115_1 = ads1115(0x49,  i2c1)
ad5272_1  = ad5272(0x2c,   i2c1)
icm20948  = icm20948(0x69, i2c0)
sht40     = sht40(0x44, i2c0)
ds3231  = ds3231(0x68, i2c0)

wiper_pos = 80

### UTILITY FUNCTIONS ###

def to_int(val) -> int:
    if val > 0x7FFF:
            val -= (0x01 << 16)

    return val

### UTILITY FUNCTIONS END ###

### ICM CONFIGURATION READOUT BEGIN ###

# whoami        = icm20948.read(MASK_REG_BANK_SEL_UB_0, 0x00)
# user_ctrl     = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_USER_CTRL)
# pwr_mgmt      = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_PWR_MGMT_1)
# pwr_mgmt_2    = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_PWR_MGMT_2)
# int_enable_2  = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_INT_ENABLE_2)
# fifo_en_2     = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_FIFO_EN_2)
# fifo_mode     = icm20948.read(MASK_REG_BANK_SEL_UB_0, ICM20948_REG_FIFO_MODE)

# smplrt_div_1  = icm20948.read(MASK_REG_BANK_SEL_UB_2, ICM20948_REG_ACCEL_SMPLRT_DIV_1)
# smplrt_div_2  = icm20948.read(MASK_REG_BANK_SEL_UB_2, ICM20948_REG_ACCEL_SMPLRT_DIV_2)
# accel_config  = icm20948.read(MASK_REG_BANK_SEL_UB_2, ICM20948_REG_ACCEL_CONFIG)

# print("      whoami: ", hex(whoami))
# print("   user_ctrl: ", hex(user_ctrl))
# print("    pwr_mgmt: ", hex(pwr_mgmt))
# print("  pwr_mgmt_2: ", hex(pwr_mgmt_2))
# print("int_enable_2: ", hex(int_enable_2))
# print("   fifo_en_2: ", hex(fifo_en_2))
# print("   fifo_mode: ", hex(fifo_mode))

# print("  smplrt_div_1: ", hex(smplrt_div_1))
# print("  smplrt_div_2: ", hex(smplrt_div_2))
# print("  accel_config: ", hex(accel_config))
# print("\n")

### ICM CONFIGURATION READOUT END ###


### ICM FIFO READOUT BEGIN ###
xa, ya, za = icm20948.read_accel()
fifo_buff, fifo_len  = icm20948.read_fifo()

print(f"x accel value: {xa}")
print(f"y accel value: {ya}")
print(f"z accel value: {za}\n")

### ICM FIFO READOUT END   ###

while True:
    ad5272_1.write_rdac(wiper_pos)

    begin = time.time()
    voltage = ads1115_1.read(ADS1115_CH.CH01.value)
    digipot = ad5272_1.read(AD5272_REG.RDAC_R.value)
    fifo_buff, fifo_len  = icm20948.read_fifo()
    sht_temp, sht_humid  = sht40.read_lpres()
    hours, minutes, sec  = ds3231.read_time()
    print(f"  DS3231 TIME: {hours}:{minutes}:{sec}")
    print(f"ADC CH0 VALUE: {voltage}")
    print(f"DIGIPOT VALUE: {digipot}")
    print(f"     FIFO LEN: {fifo_len}")
    print(f"     SHT TEMP: {sht_temp}")
    print(f"    SHT_HUMID: {sht_humid}\n")
    # wiper_pos += 1
    time.sleep(0.33)
