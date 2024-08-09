import os

# os.system("sudo mount /dev/sda1 /media/hab_flight_data")
# os.system("sudo chmod 666 /media/hab_flight_data/")

os.chdir("/home/pi/habmaster/hab")
os.system("make -f build.mak hrtim_trig_enable")

os.system("make -f build.mak setup_kernel_all HAB_KMOD_LIST=\"mprls0025 icm20x sht4x ads1115 ad5272 mlx90614\"")
os.system("make -f build.mak build_all_hab HABDEV_LIST=\"mprls0025 icm20948 sht4x ads1115_48 ads1115_49 ad5272_2c ad5272_2e ad5272_2f mlx90614 imx477_01 imx477_02\"")

os.system("sudo ../out/hab_bin/hab_master")
