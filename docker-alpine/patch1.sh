#!/bin/bash
PICO_SDK_PATH=/pico/pico-sdk

cd ${PICO_SDK_PATH}/lib/tinyusb
#git checkout master
git checkout d6354a2aa76ba1e991248faadc53734edce6eeae
git pull

git apply /0001-disable-RP2040-USB-Host-double-buffering.patch
