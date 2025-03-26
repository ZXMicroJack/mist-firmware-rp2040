#!/bin/bash
docker run --name usbhost --rm -ti -v`pwd`/..:/work picobuild3 bash
