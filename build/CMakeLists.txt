cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_MASTER -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 -DNR_PS2=2 -fsigned-char -DUSE_8BA -DSAMPLE_RATE=22050 -DSAMPLE_LEN=512 -DPICO_AUDIO_I2S_DATA_PIN=4 -DPICO_AUDIO_I2S_CLOCK_PIN_BASE=2 -DVE_STEPS=64 -DLFO_STEPS=64 -DNONAME -DPICO_AUDIO_I2S_PIO=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 -fsigned-char")

# for alpine build of docker tools
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=implicit-function-declaration -Wno-error=incompatible-pointer-types -Wno-error=int-conversion -Wno-error=return-mismatch")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

# Initialize the SDK
pico_sdk_init()

# picosynth
add_library(picosynth STATIC IMPORTED)
set_target_properties(picosynth PROPERTIES IMPORTED_LOCATION ../picosynth/libpicosynth.a)

include(example_auto_set_url.cmake)

# Build project
include_directories(. ../mist-firmware ../mist-firmware/usb .. ../contrib/tusb_xinput)

set(driver_src
  ../drivers/fpga.c
  ../drivers/pio_spi.c
  ../drivers/ps2.c
  ../drivers/ps2detect.c
  ../drivers/sdcard.c
  ../drivers/bitfile.c
  ../drivers/midi.c
  #../drivers/ipc.c
  #../drivers/flash.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/kbd.c
  ../drivers/jamma.c
  ../drivers/gpioirq.c
  ../drivers/crc16.c
)

set(driver_mb2_src
  ../drivers/fpga.c
  ../drivers/pio_spi.c
  ../drivers/sdcard.c
  ../drivers/bitfile.c
  ../drivers/ipc.c
  ../drivers/cookie.c
  ../drivers/midi.c
  #../drivers/flash.c
  ../drivers/gpioirq.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/crc16.c
  usboffshore.c
)

set(driver_zx1_src
../drivers/jtag.c
../drivers/pio_spi.c
../drivers/ps2.c
../drivers/sdcard.c
../drivers/ps2detect.c
../drivers/bitfile.c
../drivers/midi.c
#../drivers/ipc.c
#../drivers/flash.c
../drivers/gpioirq.c
../drivers/debug.c
../drivers/fifo.c
../drivers/kbd.c
#../drivers/jamma.c
../drivers/crc16.c
../drivers/bitstore.c
)

set(main_src
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  mmc.c
  rtc.c
  keyboard.c
  fpgaconfig.c
  mbconfig.c
  kbdusb.c
)

set(mainzx1_src
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  rtc.c
  mmc.c
  keyboard.c
  fpgaconfigzx1.c
  kbdusb.c
)

set(usb_src
  ../usbhost/hid_app_mist.c
  ../usbhost/msc_app.c
  ../usbhost/xinput.c
  ../contrib/tusb_xinput/xinput_host.c
  xboxusb.c
  ds3usb.c
)

set(mistfirmware_src
  ../mist-firmware/arc_file.c
  ../mist-firmware/archie.c
  ../mist-firmware/boot.c
  ../mist-firmware/cdc_control.c
  ../mist-firmware/config.c
  ../mist-firmware/cue_parser.c
  ../mist-firmware/data_io.c
  ../mist-firmware/fat_compat.c
  ../mist-firmware/fdd.c
  ../mist-firmware/font.c
  ../mist-firmware/fpga.c
  ../mist-firmware/hdd.c
  ../mist-firmware/idxfile.c
  ../mist-firmware/ikbd.c
  ../mist-firmware/ini_parser.c
  ../mist-firmware/menu-8bit.c
  ../mist-firmware/menu.c
  ../mist-firmware/menu-minimig.c
  ../mist-firmware/mist_cfg.c
  ../mist-firmware/neocd.c
  ../mist-firmware/osd.c
  ../mist-firmware/pcecd.c
  ../mist-firmware/settings.c
  ../mist-firmware/state.c
  ../mist-firmware/swap.c
  ../mist-firmware/tos.c
  ../mist-firmware/user_io.c
  ../mist-firmware/utils.c
  ../mist-firmware/xmodem.c
  ../mist-firmware/FatFs/diskio.c
  ../mist-firmware/FatFs/ff.c
  ../mist-firmware/FatFs/ffsystem.c
  ../mist-firmware/FatFs/ffunicode.c
  ../mist-firmware/usb/joymapping.c
)

set(mistfirmwareusb_src
 ../mist-firmware/usb/hidparser.c
  ../mist-firmware/usb/hid.c
  ../mist-firmware/usb/joystick.c
  ../mist-firmware/usb/timer.c
  #../mist-firmware/usb/xboxusb.c
  ../mist-firmware/usb/usbdebug.c
  #../mist-firmware/usb/pl2303.c
  )

set(picosynth_src
  picosynth.c
  ../drivers/audio_i2s.c )

set(picosynth_lib
  hardware_dma
  pico_audio
  picosynth
)

set(picosynth_opts
  "-DSAMPLE_RATE=22050"
  "-DSAMPLE_LEN=512"
  "-DPICO_AUDIO_I2S_DATA_PIN=4"
  "-DPICO_AUDIO_I2S_CLOCK_PIN_BASE=2"
  "-DPICO_AUDIO_I2S_PIO=1"
)

# mist library
add_library(mistfirmware STATIC ${mistfirmware_src})
target_compile_options(mistfirmware PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  "-DRP2040"
  "-DMIST_ROOT=\"/\""
  "-DCOREEXT=\"RBF\""
  "-DCOREEXTOTHER=\"NP1\""
)
# mist library xilinx
add_library(mistfirmwarexilinx STATIC ${mistfirmware_src})
target_compile_options(mistfirmwarexilinx PUBLIC
  "-DVDATE=\"\""
  "-DXILINX"
  "-DRP2040"
  "-DMIST"
  "-DMIST_ROOT=\"/MIST\""
  "-DCOREEXT=\"BIT\""
  "-DCOREEXTOTHER=\"ZXT\""
)

# mist USB library
add_library(mistfirmwareusbxilinx STATIC ${mistfirmware_src} ${mistfirmwareusb_src})
target_compile_options(mistfirmwareusbxilinx PUBLIC
  "-DVDATE=\"\""
  "-DRP2040"
  "-DMIST"
  "-DUSB_STORAGE"
  "-DXILINX"
  "-DEXCL_XBOX_JPAD"
  "-DMIST_ROOT=\"/MIST\""
  "-DCOREEXT=\"BIT\""
  "-DCOREEXTOTHER=\"ZXT\""
)

# mist USB library
add_library(mistfirmwareusb STATIC ${mistfirmware_src} ${mistfirmwareusb_src})
target_compile_options(mistfirmwareusb PUBLIC
  "-DVDATE=\"\""
  "-DRP2040"
  "-DMIST"
  "-DUSB_STORAGE"
  "-DEXCL_XBOX_JPAD"
  "-DMIST_ROOT=\"/\""
  "-DCOREEXT=\"RBF\""
  "-DCOREEXTOTHER=\"NP1\""
  )

# mist library
add_library(mistfirmwarezx1 STATIC ${mistfirmware_src})
target_compile_options(mistfirmwarezx1 PUBLIC
  "-DVDATE=\"\""
  "-DRP2040"
  "-DNO_FILE_INDEX"
  "-DMIST"
  "-DXILINX"
  "-DZXUNO"
  "-DMIST_ROOT=\"/MIST\""
  "-DCOREEXT=\"BIT\""
  "-DCOREEXTOTHER=\"ZX1\""
  )

  # mist library
add_library(mistfirmwareusbzx1 STATIC ${mistfirmware_src} ${mistfirmwareusb_src})
target_compile_options(mistfirmwareusbzx1 PUBLIC
  "-DVDATE=\"\""
  "-DRP2040"
  "-DNO_FILE_INDEX"
  "-DUSB_STORAGE"
  "-DEXCL_XBOX_JPAD"
  "-DMIST"
  "-DXILINX"
  "-DZXUNO"
  "-DMIST_ROOT=\"/MIST\""
  "-DCOREEXT=\"BIT\""
  "-DCOREEXTOTHER=\"ZX1\""
  )

####################################################################################
# Main build generic options
set(common_opts
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DUSB_DEBUG_OFF"
)
set(altera_opts
  "-DALTERA_FPGA"
  "-DQMTECH"
)
set(xilinx_opts
  "-DXILINX"
)
set(flash_opts
  "-DPICOSYNTH"
  "-DMIDI_NOT_I2S"
  "-DBUFFER_FPGA"
)
set(flash_opts_mb2
  "-DPICOSYNTH"
  "-DMIDI_NOT_I2S"
  "-DBUFFER_FPGA"
)
set(flash_opts_zx1
  "-DPICOSYNTH"
)
set(ram_opts
  "-DBUFFER_FPGA"
  "-DNO_FILE_INDEX"
)
set(zxtres_opts
  "-DMIST_ROOT=\"/MIST\""
  "-DSD_DIRECT_MODE_GPIO=22"
  "-DBOOT_FLASH_ON_ERROR"
)
set(neptuno_opts
  "-DSD_DIRECT_MODE_GPIO=22"
  "-DMIST_ROOT=\"/\""
)
set(zxuno_opts
  "-DNO_PS2_TX"
  "-DBOOT_FLASH_ON_ERROR"
  "-DNO_LEGACY_MODE"
  "-DSD_DIRECT_MODE_GPIO=19"
  "-DMIST_ROOT=\"/MIST\""
  "-DZXUNO"
  "-DCHECK_MIST_DIR"
)
set(usb_opts
  "-DUSB"
  "-DMIST_USB"
  "-DUSB_STORAGE"
)
set(mb2_opts
  "-DMB2"
  "-DIPC_MASTER"
)

####################################################################################
# Main build RAM test build for functionality Xilinx ZXTres
set(testarch xilinx)
# set(archdefs "-DXILINX")
# set(archdefs   "-DALTERA_FPGA" "-DQMTECH")

####################################################################################
# Main build RAM test build for functionality Xilinx ZXTres
add_executable(mistram
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  )

target_link_libraries(mistram
  pico_stdlib
  mistfirmware${testarch}
  hardware_spi
  hardware_pio
  hardware_rtc 
  pico_multicore 
  ${picosynth_lib})
add_compile_options(-O2)
target_include_directories(mistram PUBLIC ../picosynth)
pico_generate_pio_header(mistram ../drivers/fpga.pio)
pico_generate_pio_header(mistram ../drivers/spi.pio)
pico_generate_pio_header(mistram ../drivers/jamma.pio)
pico_generate_pio_header(mistram ../drivers/jammadb9.pio)
pico_generate_pio_header(mistram ../drivers/ps2.pio)
pico_generate_pio_header(mistram ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistram ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistram ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistram 1)
pico_enable_stdio_uart(mistram 0)
pico_add_extra_outputs(mistram)
example_auto_set_url(mistram)
pico_set_binary_type(mistram no_flash)

target_compile_options(mistram PUBLIC
  ${common_opts}
  ${xilinx_opts}
  # ${altera_opts}
  ${ram_opts}
  # ${neptuno_opts}
  ${zxtres_opts}
  ${picosynth_opts}
)


####################################################################################
# RAM build - for testing USB interface to USB host module.
# add_executable(mistusbfake
#   ${driver_mb2_src}
#   ${main_src}
#   ${usb_src}
#   )

# target_link_libraries(mistusbfake 
#   pico_stdlib 
#   mistfirmwareusb${testarch} 
#   hardware_spi 
#   hardware_i2c
#   pico_multicore 
#   hardware_rtc 
#   hardware_pio)
# add_compile_options(-O2)
# pico_generate_pio_header(mistusbfake ../drivers/fpga.pio)
# pico_generate_pio_header(mistusbfake ../drivers/spi.pio)
# #pico_generate_pio_header(mistusbfake ../drivers/jamma.pio)
# #pico_generate_pio_header(mistusbfake ../drivers/jammadb9.pio)
# pico_enable_stdio_usb(mistusbfake 1)
# pico_enable_stdio_uart(mistusbfake 0)
# pico_add_extra_outputs(mistusbfake)
# example_auto_set_url(mistusbfake)
# pico_set_binary_type(mistusbfake no_flash)
# target_compile_options(mistusbfake PUBLIC
# 	#  "-DVDATE=\"\""
# 	#  "-Dhexdump=driver_hexdump"
# 	#  "-DPS2HOST"
# 	#  ${archdefs}
# 	#  "-DJAMMADB9"
# 	#  "-DUSB"
# 	#  "-DUSBFAKE"
# 	#  "-DSD_DIRECT_MODE_GPIO=22"
# 	#  "-DTEST_BUILD"
# 	#  "-DNO_FILE_INDEX"
#   ${common_opts}
#   ${xilinx_opts}
#   ${ram_opts}
#   ${zxtres_opts}
#   ${mb2_opts}
#   "-DUSBFAKE"
#   "-DUSB"
# )

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistxilinx
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  )

target_link_libraries(mistxilinx 
  pico_stdlib 
  mistfirmwarexilinx 
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_rtc 
  pico_multicore 
  ${picosynth_lib}
  hardware_i2c)

  target_include_directories(mistxilinx PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistxilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistxilinx ../drivers/spi.pio)
pico_generate_pio_header(mistxilinx ../drivers/jamma.pio)
pico_generate_pio_header(mistxilinx ../drivers/jammadb9.pio)
pico_generate_pio_header(mistxilinx ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistxilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistxilinx ../drivers/ps2.pio)
pico_generate_pio_header(mistxilinx ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistxilinx 1)
pico_enable_stdio_uart(mistxilinx 0)
pico_add_extra_outputs(mistxilinx)
example_auto_set_url(mistxilinx)

target_compile_options(mistxilinx PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${flash_opts}
  ${zxtres_opts}
  ${picosynth_opts}
  )

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistusbxilinx
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  ${usb_src}
  )

target_link_libraries(mistusbxilinx 
  pico_stdlib 
  mistfirmwareusbxilinx 
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_rtc 
  hardware_i2c 
  pico_multicore 
  ${picosynth_lib}
  tinyusb_host 
  tinyusb_board)
target_include_directories(mistusbxilinx PUBLIC ../usbhost)
target_include_directories(mistusbxilinx PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistusbxilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/spi.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/jamma.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/jammadb9.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/ps2.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusbxilinx 0)
pico_enable_stdio_uart(mistusbxilinx 0)
pico_add_extra_outputs(mistusbxilinx)
example_auto_set_url(mistusbxilinx)

target_compile_options(mistusbxilinx PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${flash_opts}
  ${zxtres_opts}
  ${usb_opts}
  ${picosynth_opts}
  # "-DPIODEBUG"
  # "-DNO_JAMMA"
)

####################################################################################
# FLASH build - release binary for Altera NeptUno board -with single CPU middleboard.
add_executable(mistusb
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  ${usb_src}
  )

target_link_libraries(mistusb
  pico_stdlib 
  mistfirmwareusb 
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_i2c 
  hardware_rtc 
  pico_multicore
  ${picosynth_lib}
  tinyusb_host 
  tinyusb_board)
target_include_directories(mistusb PUBLIC ../usbhost)
target_include_directories(mistusb PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistusb ../drivers/fpga.pio)
pico_generate_pio_header(mistusb ../drivers/spi.pio)
pico_generate_pio_header(mistusb ../drivers/jamma.pio)
pico_generate_pio_header(mistusb ../drivers/jammadb9.pio)
pico_generate_pio_header(mistusb ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistusb ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistusb ../drivers/ps2.pio)
pico_generate_pio_header(mistusb ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusb 0)
pico_enable_stdio_uart(mistusb 0)
pico_add_extra_outputs(mistusb)
example_auto_set_url(mistusb)

target_compile_options(mistusb PUBLIC
  ${common_opts}
  ${altera_opts}
  ${flash_opts}
  ${neptuno_opts}
  ${usb_opts}
  ${picosynth_opts}
  )

####################################################################################
# RAM build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistmb2xilinxram
  ${driver_mb2_src}
  ${main_src}
  # ${picosynth_src}
  ${usb_src}
  )

target_link_libraries(mistmb2xilinxram 
  pico_stdlib 
  mistfirmwarexilinx 
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  # hardware_flash 
  # ${picosynth_lib}
  hardware_i2c
  pico_multicore)
  # pico_multicore tinyusb_host tinyusb_board)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2xilinxram ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2xilinxram ../drivers/spi.pio)
pico_generate_pio_header(mistmb2xilinxram ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistmb2xilinxram ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistmb2xilinxram ../drivers/uart_tx.pio)
# NOUSB
pico_enable_stdio_usb(mistmb2xilinxram 1)
# USB
# target_include_directories(mistmb2xilinxram PUBLIC ../usbhost)
# pico_enable_stdio_usb(mistmb2xilinxram 0)

target_include_directories(mistmb2xilinxram PUBLIC ../picosynth)

pico_enable_stdio_uart(mistmb2xilinxram 0)
pico_add_extra_outputs(mistmb2xilinxram)
example_auto_set_url(mistmb2xilinxram)
pico_set_binary_type(mistmb2xilinxram no_flash)
target_compile_options(mistmb2xilinxram PUBLIC
  ${common_opts}
  ${ram_opts}
  ${xilinx_opts}
  ${zxtres_opts}
  ${mb2_opts}
  )

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistmb2xilinx
  ${driver_mb2_src}
  ${main_src}
  ${usb_src}
  ${picosynth_src}
  )

# mistfirmwareusbxilinx
target_link_libraries(mistmb2xilinx 
  pico_stdlib
  mistfirmwareusbxilinx
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_rtc 
  hardware_i2c 
  ${picosynth_lib}
  pico_multicore tinyusb_host tinyusb_board)
target_include_directories(mistmb2xilinx PUBLIC ../usbhost)
target_include_directories(mistmb2xilinx PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2xilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/spi.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/uart_tx.pio)
pico_enable_stdio_usb(mistmb2xilinx 0)
pico_enable_stdio_uart(mistmb2xilinx 0)
pico_add_extra_outputs(mistmb2xilinx)
example_auto_set_url(mistmb2xilinx)
set_target_properties(mistmb2xilinx PROPERTIES PICO_TARGET_LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/standalone.ld")
target_compile_options(mistmb2xilinx PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${flash_opts_mb2}
  ${zxtres_opts}
  ${mb2_opts}
  ${usb_opts}
  ${picosynth_opts}
  # "-DPIODEBUG"
  )

####################################################################################
# FLASH build - release binary for Altera NeptUno board -with single CPU middleboard.
add_executable(mistusbn1p
  ${driver_src}
  ${main_src}
  ${usb_src}
  ${picosynth_src}
  )

target_link_libraries(mistusbn1p
  pico_stdlib 
  mistfirmwareusb 
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_i2c 
  hardware_rtc 
  pico_multicore
  tinyusb_host 
  tinyusb_board
  ${picosynth_lib}
  )
target_include_directories(mistusbn1p PUBLIC ../usbhost)
target_include_directories(mistusbn1p PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistusbn1p ../drivers/fpga.pio)
pico_generate_pio_header(mistusbn1p ../drivers/spi.pio)
pico_generate_pio_header(mistusbn1p ../drivers/jamma.pio)
pico_generate_pio_header(mistusbn1p ../drivers/jammadb9.pio)
pico_generate_pio_header(mistusbn1p ../drivers/ps2.pio)
pico_generate_pio_header(mistusbn1p ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusbn1p 0)
pico_enable_stdio_uart(mistusbn1p 0)
pico_add_extra_outputs(mistusbn1p)
example_auto_set_url(mistusbn1p)
#pico_set_binary_type(mistusb no_flash)
target_compile_options(mistusbn1p PUBLIC
  ${common_opts}
  ${altera_opts}
  ${flash_opts}
  ${neptuno_opts}
  ${usb_opts}
  ${picosynth_opts}
  )

####################################################################################
# Main build RAM test build for ZXUno
add_executable(mistzx1ram
  ${driver_zx1_src}
  ${mainzx1_src}
  ${picosynth_src}
  )

target_link_libraries(mistzx1ram 
  pico_stdlib 
  mistfirmwarezx1 
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  pico_multicore 
  hardware_i2c 
  ${picosynth_lib})
  
add_compile_options(-O2)
target_include_directories(mistzx1ram PUBLIC ../picosynth)
pico_generate_pio_header(mistzx1ram ../drivers/spi.pio)
pico_generate_pio_header(mistzx1ram ../drivers/ps2.pio)
pico_generate_pio_header(mistzx1ram ../drivers/audio_i2s_zx1.pio)
pico_generate_pio_header(mistzx1ram ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistzx1ram 1)
pico_enable_stdio_uart(mistzx1ram 0)
pico_add_extra_outputs(mistzx1ram)
example_auto_set_url(mistzx1ram)
pico_set_binary_type(mistzx1ram no_flash)
target_compile_options(mistzx1ram PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${ram_opts}
  ${zxuno_opts}
  ${picosynth_opts}
)

####################################################################################
# Main build RAM test build for ZXUno
add_executable(mistzx1
  ${driver_zx1_src}
  ${mainzx1_src}
  ${picosynth_src}
  )

target_link_libraries(mistzx1
  pico_stdlib 
  mistfirmwarezx1
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  pico_multicore 
  ${picosynth_lib})
add_compile_options(-O2)
target_include_directories(mistzx1 PUBLIC ../picosynth)
pico_generate_pio_header(mistzx1 ../drivers/spi.pio)
pico_generate_pio_header(mistzx1 ../drivers/ps2.pio)
pico_generate_pio_header(mistzx1 ../drivers/audio_i2s_zx1.pio)
pico_generate_pio_header(mistzx1 ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistzx1 1)
pico_enable_stdio_uart(mistzx1 0)
pico_add_extra_outputs(mistzx1)
example_auto_set_url(mistzx1)
target_compile_options(mistzx1 PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${flash_opts_zx1}
  ${zxuno_opts}
  ${picosynth_opts}
)

####################################################################################
# Main build RAM test build for ZXUno
add_executable(mistusbzx1
  ${driver_zx1_src}
  ${mainzx1_src}
  ${usb_src}
  ${picosynth_src}
  )

target_link_libraries(mistusbzx1 
  pico_stdlib 
  mistfirmwareusbzx1 
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  pico_multicore 
  tinyusb_host 
  tinyusb_board
  ${picosynth_lib}
)
  # ${picosynth_lib})
add_compile_options(-O2)
target_include_directories(mistusbzx1 PUBLIC ../picosynth)
target_include_directories(mistusbzx1 PUBLIC ../usbhost)
pico_generate_pio_header(mistusbzx1 ../drivers/spi.pio)
pico_generate_pio_header(mistusbzx1 ../drivers/ps2.pio)
pico_generate_pio_header(mistusbzx1 ../drivers/audio_i2s_zx1.pio)
pico_generate_pio_header(mistusbzx1 ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusbzx1 0)
pico_enable_stdio_uart(mistusbzx1 0)
pico_add_extra_outputs(mistusbzx1)
example_auto_set_url(mistusbzx1)

target_compile_options(mistusbzx1 PUBLIC
  ${common_opts}
  ${xilinx_opts}
  ${flash_opts_zx1}
  ${zxuno_opts}
  ${usb_opts}
  ${picosynth_opts}
  )

####################################################################################
# RAM build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistmb2ram
  ${driver_mb2_src}
  ${main_src}
  ${usb_src}
  )

target_link_libraries(mistmb2ram 
  pico_stdlib 
  mistfirmware
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  hardware_i2c
  pico_multicore)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2ram ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2ram ../drivers/spi.pio)
pico_generate_pio_header(mistmb2ram ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistmb2ram ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistmb2ram ../drivers/uart_tx.pio)
pico_enable_stdio_usb(mistmb2ram 1)

target_include_directories(mistmb2ram PUBLIC ../picosynth)

pico_enable_stdio_uart(mistmb2ram 0)
pico_add_extra_outputs(mistmb2ram)
example_auto_set_url(mistmb2ram)
pico_set_binary_type(mistmb2ram no_flash)
target_compile_options(mistmb2ram PUBLIC
  ${common_opts}
  ${altera_opts}
  ${ram_opts}
  ${neptuno_opts}
  ${mb2_opts}
  )



####################################################################################
# FLASH build - release binary for Neptuno with dual mb
add_executable(mistmb2
  ${driver_mb2_src}
  ${main_src}
  ${usb_src}
  ${picosynth_src}
  )

# mistfirmwareusbxilinx
target_link_libraries(mistmb2 
  pico_stdlib
  mistfirmwareusb
  hardware_spi 
  hardware_pio 
  hardware_flash 
  hardware_rtc 
  hardware_i2c 
  ${picosynth_lib}
  pico_multicore tinyusb_host tinyusb_board)
target_include_directories(mistmb2 PUBLIC ../usbhost)
target_include_directories(mistmb2 PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2 ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2 ../drivers/spi.pio)
pico_generate_pio_header(mistmb2 ../drivers/audio_not_i2s.pio)
pico_generate_pio_header(mistmb2 ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistmb2 ../drivers/uart_tx.pio)
pico_enable_stdio_usb(mistmb2 0)
pico_enable_stdio_uart(mistmb2 0)
pico_add_extra_outputs(mistmb2)
example_auto_set_url(mistmb2)
set_target_properties(mistmb2 PROPERTIES PICO_TARGET_LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/standalone.ld")
target_compile_options(mistmb2 PUBLIC
  ${common_opts}
  ${altera_opts}
  ${flash_opts_mb2}
  ${neptuno_opts}
  ${mb2_opts}
  ${usb_opts}
  ${picosynth_opts}
  # "-DPIODEBUG"
  )


####################################################################################
# RAM build - test picosynth
add_executable(miditest
  ${driver_mb2_src}
  miditest.c
  ${picosynth_src}
  )

target_link_libraries(miditest 
  pico_stdlib 
  mistfirmware
  hardware_spi 
  hardware_pio 
  hardware_rtc 
  hardware_i2c
  pico_multicore
  ${picosynth_lib})
add_compile_options(-O2)
pico_generate_pio_header(miditest ../drivers/audio_i2s.pio)
pico_generate_pio_header(miditest ../drivers/audio_not_i2s.pio)
pico_enable_stdio_usb(miditest 1)

target_include_directories(miditest PUBLIC ../picosynth)

pico_enable_stdio_uart(miditest 0)
pico_add_extra_outputs(miditest)
example_auto_set_url(miditest)
pico_set_binary_type(miditest no_flash)
target_compile_options(miditest PUBLIC 
  ${picosynth_opts}
  #  "-DMIDI_NOT_I2S"
  )



