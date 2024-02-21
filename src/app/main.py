import board
import busio
import time

from ads111x.ads1115 import ads1115
from ads111x import ADS1115_CH

from ad527x.ad5272 import ad5272
from ad527x import AD5272_REG

i2c0 = busio.I2C(board.D1, board.D0)
i2c1 = busio.I2C(board.D3, board.D2)

ads1115_1 = ads1115(0x49, i2c1)
ad5272_1  = ad5272(0x2c,  i2c1)

wiper_pos = 1

while True:
    ad5272_1.write_rdac(wiper_pos)

    a = ads1115_1.read(ADS1115_CH.CH0.value)
    digipot = ad5272_1.read(AD5272_REG.RDAC_R.value)

    print("ADC CH0 VALUE: ", a[0] << 8 | a[1])
    print(f"DIGIPOT VALUE: {digipot}\n")

    wiper_pos += 1
    time.sleep(1)
