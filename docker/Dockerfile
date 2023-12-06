FROM ubuntu
COPY pico_setup.sh .

RUN echo \$@ > /bin/sudo && chmod a+x /bin/sudo
RUN DEBIAN_FRONTEND=noninteractive ./pico_setup.sh

RUN apt-get install -y python3 mc vim
CMD bash

