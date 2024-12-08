#!/bin/bash
if [ "$1" != "" ]; then
docker run --rm -ti -v`pwd`/..:/work picobuild2 $1
else
docker run --rm -ti -v`pwd`/..:/work picobuild2 bash -c "cd /work/drivertest; make"
fi
