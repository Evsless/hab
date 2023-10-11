import w1thermsensor
import time
import os

sensor = w1thermsensor.W1ThermSensor()


def main():
    while(True):
        print(sensor.get_temperature())
        time.sleep(0.1)
        # os.system('clear')

main()
