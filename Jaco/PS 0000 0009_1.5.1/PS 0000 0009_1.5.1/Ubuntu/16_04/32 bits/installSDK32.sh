#!/bin/bash
echo 'KINOVA SDK INSTALLATION'
echo 'This operation require root privilege.'

sudo rm /usr/lib/*Kinova*.so > null

sudo dpkg -i KinovaAPI-6.1.0-i386.deb

sudo apt-get install -y libqt5webkit5 libqt5xml5

chmod +x ./SDK_GEN2_1_5_1_install32

sudo ./SDK_GEN2_1_5_1_install32

