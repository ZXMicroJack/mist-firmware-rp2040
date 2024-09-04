#!/bin/bash
docker run --rm -ti -v`pwd`:/work picobuild2 bash -c "cd /work/build2; make -j8 $1"
docker run --rm -ti -v`pwd`:/work picobuild2 bash -c "cd /work/rp2u-build; make -j8 $1"
