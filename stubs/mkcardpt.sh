#!/bin/bash

set -e

dd if=/dev/zero of=card.pt bs=1024 count=131072

fdisk card.pt << EOF
n
p
1

+60M
t
0c
n
p
2


t
2
06
w
EOF

sudo losetup -P /dev/loop55 card.pt
#sudo mkfs.vfat -F 32 -s 2 /dev/loop55p1
sudo mkfs.vfat -F 32 /dev/loop55p1
sudo mkfs.vfat -F 16 /dev/loop55p2


mkdir d || true
sudo mount /dev/loop55p1 ./d
sudo chmod -R 777 d

sudo cp -r ../card/neptuno2/* ./d
find d
sudo umount d

rm -rd d

chmod 777 card.pt

# write to card
#sudo dd if=card of=/dev/mmcblk0

#sync
#sync
sudo losetup -d /dev/loop55
