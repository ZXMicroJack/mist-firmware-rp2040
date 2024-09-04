#!/bin/bash
docker run --rm -ti -v`pwd`:/work picobuilda bash -c "cd /work/build2; make -j8 $1"
docker run --rm -ti -v`pwd`:/work picobuilda bash -c "cd /work/rp2u-build; make -j8 $1"
