cmd_/home/pi/linux-device-driver/01_arg_parser/Module.symvers := sed 's/ko$$/o/' /home/pi/linux-device-driver/01_arg_parser/modules.order | scripts/mod/modpost -m -a   -o /home/pi/linux-device-driver/01_arg_parser/Module.symvers -e -i Module.symvers   -T -
