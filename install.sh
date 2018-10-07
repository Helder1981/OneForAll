#!/bin/bash


cd ~
sudo chmod 777 /home/pi/OneForAll/osd/osd
sudo chmod 777 /home/pi/OneForAll/rfkill/rfkill
sudo chmod 777 /home/pi/OneForAll/monitor
sudo sed -i '/\"exit 0\"/!s/exit 0/\/home\/pi\/OneForAll\/monitor \&\nexit 0/g' /etc/rc.local

config_txt=/boot/config.txt
echo "Enabling i2c..."
if ! grep '^dtparam=i2c_arm=on' $config_txt; then
  echo 'dtparam=i2c_arm=on' >> $config_txt
else
  echo "i2c already enabled."
fi

etc_modules=/etc/modules
echo "Adding entries to $etc_modules..."
if ! grep '^i2c-bcm2708' $etc_modules; then
  echo 'i2c-bcm2708' >> $etc_modules
  echo 'i2c-dev' >> $etc_modules
else
  echo "$etc_modules already set up."
fi
