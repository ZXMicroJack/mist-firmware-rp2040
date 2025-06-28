#!/bin/bash
if [ "$1" != "" ]; then
docker run --rm -ti -v`pwd`/..:/work picobuild3 $1
else
docker run --rm -ti -v`pwd`/..:/work picobuild3 bash -c "cd /work/usbhost; make"
fi
