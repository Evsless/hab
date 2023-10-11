import time
import board
import adafruit_mprls

i2c = board.I2C()

# Simplest use, connect to default over I2C
mpr = adafruit_mprls.MPRLS(i2c, psi_min=0, psi_max=25)

while True:
    print((mpr.pressure,))
    time.sleep(1)