import board
import busio

import time
from ads111x.ads1115 import ads1115
from ads111x import ADS1115_CH

from adafruit_bus_device.i2c_device import I2CDevice

i2c0 = busio.I2C(board.D1, board.D0)
i2c1 = busio.I2C(board.D3, board.D2)

ads1115_1 = ads1115(0x49, i2c1)

while True:
    a = ads1115_1.read(ADS1115_CH.CH0.value)
    print(a[0] << 8 | a[1])
    time.sleep(1)
