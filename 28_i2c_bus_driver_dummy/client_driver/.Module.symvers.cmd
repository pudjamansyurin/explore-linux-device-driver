cmd_/home/pi/linux-device-driver/28_i2c_bus_driver_dummy/client_driver/Module.symvers := sed 's/ko$$/o/' /home/pi/linux-device-driver/28_i2c_bus_driver_dummy/client_driver/modules.order | scripts/mod/modpost -m -a   -o /home/pi/linux-device-driver/28_i2c_bus_driver_dummy/client_driver/Module.symvers -e -i Module.symvers   -T -