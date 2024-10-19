FROM alpine
COPY pico_setup.sh .

RUN echo \$@ > /bin/sudo && chmod a+x /bin/sudo

RUN apk update && apk add bash
RUN apk add cmake gcc-arm-none-eabi gcc g++ python3
RUN apk add gdb-multiarch automake autoconf make texinfo libtool
RUN apk add git

ENV GITHUB_PREFIX="https://github.com/raspberrypi/"
ENV GITHUB_SUFFIX=".git"
#ENV SDK_BRANCH="master"
ENV SDK_BRANCH="1.5.1"
ENV SDK_BRANCH2="sdk-1.5.1"


#for REPO in sdk examples extras playground

#RUN git clone --recurse-submodules --shallow-submodules -b $SDK_BRANCH ${GITHUB_PREFIX}pico-sdk${GITHUB_SUFFIX} ./pico/pico-sdk
RUN git clone -b $SDK_BRANCH ${GITHUB_PREFIX}pico-sdk${GITHUB_SUFFIX} ./pico/pico-sdk && cd ./pico/pico-sdk && git submodule update --init
RUN git clone -b $SDK_BRANCH2 ${GITHUB_PREFIX}pico-examples${GITHUB_SUFFIX} ./pico/pico-examples && cd ./pico/pico-examples && git submodule update --init
RUN git clone -b $SDK_BRANCH2 ${GITHUB_PREFIX}pico-extras${GITHUB_SUFFIX} ./pico/pico-extras && cd ./pico/pico-extras && git submodule update --init
RUN git clone -b $SDK_BRANCH2 ${GITHUB_PREFIX}pico-playground${GITHUB_SUFFIX} ./pico/pico-playground && cd ./pico/pico-playground && git submodule update --init

# || git clone -b sdk-$SDK_BRANCH $REPO_URL




#RUN DEBIAN_FRONTEND=noninteractive ./pico_setup.sh

RUN echo "export PICO_SDK_PATH=/pico/pico-sdk" >> ~/.bashrc
RUN echo "export PICO_EXAMPLES_PATH=/pico/pico-examples" >> ~/.bashrc
RUN echo "export PICO_EXTRAS_PATH=/pico/pico-extras" >> ~/.bashrc
RUN echo "export PICO_PLAYGROUND_PATH=/pico/pico-playground" >> ~/.bashrc


RUN apk add mc vim
RUN apk add g++-arm-none-eabi
# XINPUT patch
COPY usbh.c /pico/pico-sdk/lib/tinyusb/src/host/usbh.c
CMD bash

