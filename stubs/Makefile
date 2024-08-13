default: mist-emu
#mist-firmware 

# fat.o \

commonhwobjs=\
hardware.o \
xboxusb.o

hwobjs=\
mmc.o \
glue.o \
spi.o \
usbstub.o \
usbdev.o

emuobjs=\
mmcemu.o \
glueemu.o \
spiemu.o \
screen.o \
usbdevx.o \
usbemu.o \
hid.o \
hidparser.o \
joystick.o \
timer.o \
usbdebug.o \
usbfake.o

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
font.o \
fpga.o \
hdd.o \
idxfile.o \
ikbd.o \
ini_parser.o \
menu-8bit.o \
menu.o \
menu-minimig.o \
mist_cfg.o \
neocd.o \
main.o \
osd.o \
pcecd.o \
settings.o \
state.o \
swap.o \
tos.o \
user_io.o \
utils.o \
xmodem.o \
diskio.o \
ff.o \
ffsystem.o \
ffunicode.o \
joymapping.o

objs1=$(hwobjs) $(commonhwobjs) $(mistobjs)
objs2=$(emuobjs) $(commonhwobjs) $(mistobjs) 
objs=$(hwobjs) $(emuobjs) $(commonhwobjs) $(mistobjs)

# syscalls.o \

copts=-m32 -I. -I../mist-firmware -I../mist-firmware/arch -I../mist-firmware/usb -I../usbhost -I../contrib/tusb_xinput/ -I..\
	-D_WANT_IO_LONG_LONG -DUSB -DUSBFAKE \
	-DVDATE="" -DRAMFUNC= -DATTRS_H -DFAST= -DMIST_ROOT=\"\" -DNOSPIN -DCOREEXT=\"BIT\" \
	-DCOREEXTOTHER=\"RBF\" -g -DEXCL_XBOX_JPAD -DTU_ATTR_WEAK= -Dxfer_result_t=int

%.o : %.c
	gcc $(copts) -c -o$@ $<

%.o : ../build/%.c
	gcc $(copts) -c -o$@ $<

%.o : ../mist-firmware/%.c
	gcc $(copts) -c -o$@ $<

%.o : ../mist-firmware/FatFs/%.c
	gcc $(copts) -c -o$@ $<

%.o : ../mist-firmware/usb/%.c
	gcc $(copts) -c -o$@ $<

%.d : ../mist-firmware/%.c
	gcc $(copts) -M -o$@ $<

%.d : ../mist-firmware/FatFs/%.c
	gcc $(copts) -M -o$@ $<

%.d : ../mist-firmware/usb/%.c
	gcc $(copts) -M -o$@ $<

%.d : %.c
	gcc $(copts) -M -o$@ $<

%.d : ../build/%.c
	gcc $(copts) -M -o$@ $<

include $(objs:.o=.d)

mist-firmware: $(objs1)
	gcc -m32 -g -o$@ $(objs1)

mist-emu: $(objs2)
	gcc -m32 -g -o$@ $(objs2) -lSDL

clean:
	-rm *.o *.d mist-firmware mist-emu
