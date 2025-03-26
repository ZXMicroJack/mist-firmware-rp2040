FROM ubuntu
COPY pico_setup.sh .

RUN echo \$@ > /bin/sudo && chmod a+x /bin/sudo
RUN DEBIAN_FRONTEND=noninteractive ./pico_setup.sh

RUN apt-get install -y python3 mc vim

# XINPUT patch
RUN cd /pico/pico-sdk/lib/tinyusb/ && git fetch && git checkout 0.18.0
COPY usbh.c /pico/pico-sdk/lib/tinyusb/src/host/usbh.c
COPY cdc /pico/pico-sdk/lib/tinyusb/src/class/cdc

CMD bash

