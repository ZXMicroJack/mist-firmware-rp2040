#!/bin/bash
setversion()
{
version=$1$2
}

source ./build/version.h

rm -rf ./release

mkdir release

#################################
# NEPTUNO+ (NEPTUNO2)
mkdir release/neptuno_plus
cp ./build2/mistusb.uf2 ./release/neptuno_plus
cd ./release/neptuno_plus
zip ../mist-neptuno-${version}.zip mistusb.uf2
zip ../mist-neptuno2-${version}.zip mistusb.uf2
cd -

#################################
# ZXTRES MB1
mkdir release/zxtres_mb
cp ./build2/mistusbxilinx.uf2 ./release/zxtres_mb
cd ./release/zxtres_mb
zip ../mist-zx3-fw-only-mb1-${version}.zip *
cd -

#################################
# ZXTRES MB2
mkdir release/zxtres_mb2
mkdir release/zxtres_mb2_test
./tools/mkimage -v ${version} -m -i ./build2/mistmb2xilinx.bin -o ./release/zxtres_mb2/RP2MAPP.BIN
./tools/mkuf2 ./release/zxtres_mb2/rp2m-signed.uf2 ./release/zxtres_mb2/RP2MAPP.BIN 1000F000

source ./rp2u-build/version.h
./tools/mkimage -v ${version} -i ./rp2u-build/rp2u.bin -o ./release/zxtres_mb2/RP2UAPP.BIN
./tools/mkuf2 ./release/zxtres_mb2/rp2u-signed.uf2 ./release/zxtres_mb2/RP2UAPP.BIN 1000F000

./tools/mkimage -v ${version} -i ./rp2u-build/rp2u_nousb.bin -o ./release/zxtres_mb2_test/RP2UAP2.BIN
./tools/mkuf2 ./release/zxtres_mb2_test/rp2u-signed.uf2 ./release/zxtres_mb2_test/RP2UAP2.BIN 1000F000

cd ./release/zxtres_mb2
zip ../mist-zx3-fw-only-mb2-${version}.zip RP2?APP.BIN *.uf2
cd -

#################################
# NEPTUNO2 MB2
mkdir release/neptuno_mb2
mkdir release/neptuno_mb2_test
./tools/mkimage -v ${version} -m -i ./build2/mistmb2.bin -o ./release/neptuno_mb2/RP2MAPP.BIN
./tools/mkuf2 ./release/neptuno_mb2/rp2m-signed.uf2 ./release/neptuno_mb2/RP2MAPP.BIN 1000F000

source ./rp2u-build/version.h
./tools/mkimage -v ${version} -i ./rp2u-build/rp2u.bin -o ./release/neptuno_mb2/RP2UAPP.BIN
./tools/mkuf2 ./release/neptuno_mb2/rp2u-signed.uf2 ./release/neptuno_mb2/RP2UAPP.BIN 1000F000

./tools/mkimage -v ${version} -i ./rp2u-build/rp2u_nousb.bin -o ./release/neptuno_mb2_test/RP2UAP2.BIN
./tools/mkuf2 ./release/neptuno_mb2_test/rp2u-signed.uf2 ./release/neptuno_mb2_test/RP2UAP2.BIN 1000F000

cd ./release/neptuno_mb2
zip ../mist-neptuno-fw-only-mb2-${version}.zip RP2?APP.BIN *.uf2
cd -


#################################
# ZXUNO
mkdir release/zxuno_plus
cp ./build2/mistusbzx1.uf2 ./release/zxuno_plus
cp ./build2/mistzx1.uf2 ./release/zxuno_plus/mistzx1_nousb.uf2

# EDIT HERE
# for when menu is unchanged
#zip ../release/zxuno_plus/mist-zxuno-${version}.zip mistusbzx1.uf2
# for when menu is changed
cp ./files/zxuno/menu.uf2 ./release/zxuno_plus/
cd ./release/zxuno_plus
zip ../mist-zxuno-${version}.zip mistusbzx1.uf2 menu.uf2
cd -
# EDIT ENDS

#################################
# PICOSYNTH
#mkdir release/picosynth

#################################
# DEBRICK
mkdir release/debrick
cd release/debrick
unzip ../../files/debrick-0203.zip
cd ..

mkdir -p ../release/debrick/neptuno_mb2/rp2midi
mkdir -p ../release/debrick/neptuno_mb2/rp2usb
cp ../release/neptuno_mb2/rp2m-signed.uf2 ../release/debrick/neptuno_mb2/rp2midi/rp2m-signed.uf2
cp ../release/neptuno_mb2/rp2u-signed.uf2 ../release/debrick/neptuno_mb2/rp2usb/rp2u-signed.uf2
cp ../release/debrick/zx3mb2/rp2midi/rp2m-bootstrap.uf2 ../release/debrick/neptuno_mb2/rp2midi/
cp ../release/debrick/zx3mb2/rp2usb/rp2u-bootstrap.uf2 ../release/debrick/neptuno_mb2/rp2usb/
cp ../build2/mistusbxilinx.uf2 ../release/debrick/zx3mb1/mistusbxilinx.uf2
cp ../build2/mistusb.uf2 ../release/debrick/neptuno/mistusb.uf2
cp ../release/zxtres_mb2/rp2m-signed.uf2 ../release/debrick/zx3mb2/rp2midi/rp2m-signed.uf2
cp ../release/zxtres_mb2/rp2u-signed.uf2 ../release/debrick/zx3mb2/rp2usb/rp2u-signed.uf2
mkdir ../release/debrick/zxuno_plus
cp ../files/zxuno/menu.uf2 ../release/debrick/zxuno_plus
cp ../build2/mistusbzx1.uf2 ../release/debrick/zxuno_plus/mistusbzx1.uf2
cp ../build2/mistzx1.uf2 ../release/debrick/zxuno_plus/mistzx1_nousb.uf2

cd ./debrick
zip -r ../debrick-${version}.zip *
cd ..

#################################
# ALL ARTEFACTS
if [ "$1" == "" ]; then
	echo stopping before packageing.....  type "$1 go" to do the full thing.
	exit 1
fi

zip -r ../releases/rel-${version}.zip .
exit

dontdo()
{
cp ../release/zxtres_mb/mistusbxilinx.uf2 .
cd ./release/zxtres_mb/


cp ../release/zxtres_mb2/RP2UAPP.BIN .
cp ../release/zxtres_mb2/RP2MAPP.BIN .
zip ../release/zxtres_mb/mist-zx3-fw-only-mb2-${version}.zip RP2?APP.BIN

cp ../release/neptuno_plus/mistusb.uf2 .
zip ../release/neptuno_plus/mist-neptuno-${version}.zip mistusb.uf2
zip ../release/neptuno_plus/mist-neptuno2-${version}.zip mistusb.uf2

zip -r ../releases/rel-${version}.zip .

cd ../release

mkdir files/common

zip -r ../releases/rel-${version}.zip .
}



dontdo()
{
rm -rf ./temp
mkdir ./temp
cd ./temp
cp -r ../package/common/* .
cp -r ../package/a35t/* .

cp ../release/zxtres_mb2/RP2UAPP.BIN ./mb2migr8
cp ../release/zxtres_mb2/RP2MAPP.BIN ./mb2migr8

zip -r ../release/zxtres_mb2/mist-zx3-a35t-mb2-${version}.zip *

cp -r ../package/a200t/* .
zip -r ../release/zxtres_mb2/mist-zx3-a200t-mb2-${version}.zip *

cp -r ../package/a100t/* .
zip -r ../release/zxtres_mb2/mist-zx3-a100t-mb2-${version}.zip *
}

