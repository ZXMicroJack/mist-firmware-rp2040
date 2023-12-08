default: mist-firmware

# fat.o \

hwobjs=\
spi.o \
usbdev.o \
mmc.o \
glue.o \
hardware.o

mistobjs=\
arc_file.o \
archie.o \
boot.o \
cdc_control.o \
config.o \
cue_parser.o \
data_io.o \
fat_compat.o \
fdd.o \
firmware.o \
font.o \
fpga.o \
hdd.o \
idxfile.o \
ikbd.o \
ini_parser.o \
main.o \
menu-8bit.o \
menu.o \
menu-minimig.o \
mist_cfg.o \
neocd.o \
osd.o \
pcecd.o \
settings.o \
state.o \
storage_control.o \
swap.o \
tos.o \
user_io.o \
utils.o \
xmodem.o \
diskio.o \
ff.o \
ffsystem.o \
ffunicode.o \
joystick.o \
joymapping.o \
usb.o \
usbdebug.o \
timer.o


objs=$(hwobjs) $(mistobjs)


# syscalls.o \


copts=-I../mist-firmware -I. -I../mist-firmware/arch -D_WANT_IO_LONG_LONG -DVDATE=""

%.o : ../mist-firmware/%.c
	gcc $(copts) -c -o$@ $<

%.o : ../mist-firmware/FatFs/%.c
	gcc $(copts) -c -o$@ $<

%.o : ../mist-firmware/usb/%.c
	gcc $(copts) -c -o$@ $<

%.o : %.c
	gcc $(copts) -c -o$@ $<

%.d : ../mist-firmware/%.c
	gcc $(copts) -M -o$@ $<

%.d : ../mist-firmware/FatFs/%.c
	gcc $(copts) -M -o$@ $<

%.d : ../mist-firmware/usb/%.c
	gcc $(copts) -M -o$@ $<

%.d : %.c
	gcc $(copts) -M -o$@ $<

include $(objs:.o=.d)

mist-firmware: $(objs)
	gcc -o$@ $(objs)

clean:
	-rm *.o *.d mist-firmware
