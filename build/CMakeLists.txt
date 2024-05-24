cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
# include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_MASTER -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 -DNR_PS2=2 -fsigned-char -DSD_DIRECT_MODE_GPIO=22 -DUSE_8BA -DSAMPLE_RATE=22050 -DSAMPLE_LEN=512 -DPICO_AUDIO_I2S_DATA_PIN=4 -DPICO_AUDIO_I2S_CLOCK_PIN_BASE=2 -DVE_STEPS=64 -DLFO_STEPS=64 -DNONAME -DPICO_AUDIO_I2S_PIO=1")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")

# Initialize the SDK
pico_sdk_init()

# picosynth
add_library(picosynth STATIC IMPORTED)
set_target_properties(picosynth PROPERTIES IMPORTED_LOCATION ../picosynth/libpicosynth.a)

include(example_auto_set_url.cmake)

# Build project
include_directories(. ../mist-firmware ../mist-firmware/usb ..)

set(driver_src
  ../drivers/fpga.c
  ../drivers/pio_spi.c
  ../drivers/ps2.c
  ../drivers/sdcard.c
  ../drivers/bitfile.c
  ../drivers/midi.c
  #../drivers/ipc.c
  #../drivers/flash.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/kbd.c
  ../drivers/jamma.c
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
../drivers/bitfile.c
../drivers/midi.c
#../drivers/ipc.c
#../drivers/flash.c
../drivers/debug.c
../drivers/fifo.c
../drivers/kbd.c
#../drivers/jamma.c
../drivers/crc16.c
)

set(main_src
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  mmc.c
  keyboard.c
  fpgaconfig.c
)

set(mainzx1_src
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  mmc.c
  keyboard.c
  fpgaconfigzx1.c
)

set(usb_src
  ../usbhost/hid_app_mist.c
  ../usbhost/msc_app.c
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
  hardware_i2c 
  picosynth
)


# mist library
add_library(mistfirmware STATIC ${mistfirmware_src})
target_compile_options(mistfirmware PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  "-DMIST_ROOT=\"/\""
)

# mist library xilinx
add_library(mistfirmwarexilinx STATIC ${mistfirmware_src})
target_compile_options(mistfirmwarexilinx PUBLIC
  "-DVDATE=\"\""
  "-DXILINX"
  "-DMIST"
  "-DMIST_ROOT=\"/MIST\""
)

# mist USB library
add_library(mistfirmwareusbxilinx STATIC ${mistfirmware_src} ${mistfirmwareusb_src})
target_compile_options(mistfirmwareusbxilinx PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  "-DUSB_STORAGE"
  "-DXILINX"
  "-DEXCL_XBOX_JPAD"
  "-DMIST_ROOT=\"/MIST\""
)

# mist USB library
add_library(mistfirmwareusb STATIC ${mistfirmware_src} ${mistfirmwareusb_src})
target_compile_options(mistfirmwareusb PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  "-DUSB_STORAGE"
  "-DEXCL_XBOX_JPAD"
  "-DMIST_ROOT=\"/\""
)

####################################################################################
# Main build RAM test build for functionality Xilinx ZXTres
# set(testarch xilinx)
# set(archdefs "-DXILINX")
set(archdefs   "-DALTERA_FPGA" "-DQMTECH")

####################################################################################
# Main build RAM test build for functionality Xilinx ZXTres
add_executable(mistram
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  )

target_link_libraries(mistram pico_stdlib mistfirmware${testarch} hardware_spi hardware_pio pico_multicore 
${picosynth_lib})
add_compile_options(-O2)
target_include_directories(mistram PUBLIC ../picosynth)
pico_generate_pio_header(mistram ../drivers/fpga.pio)
pico_generate_pio_header(mistram ../drivers/spi.pio)
pico_generate_pio_header(mistram ../drivers/jamma.pio)
pico_generate_pio_header(mistram ../drivers/jammadb9.pio)
pico_generate_pio_header(mistram ../drivers/ps2.pio)
pico_generate_pio_header(mistram ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistram ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistram 1)
pico_enable_stdio_uart(mistram 0)
pico_add_extra_outputs(mistram)
example_auto_set_url(mistram)
pico_set_binary_type(mistram no_flash)
target_compile_options(mistram PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  ${archdefs}
  "-DJAMMADB9"
  # "-DPICOSYNTH"
  #"-DTEST_BUILD"
)


####################################################################################
# RAM build - for testing USB interface to USB host module.
add_executable(mistusbfake
  ${driver_src}
  ${main_src}
  )

target_link_libraries(mistusbfake pico_stdlib mistfirmwareusb${testarch} hardware_spi hardware_pio)
add_compile_options(-O2)
pico_generate_pio_header(mistusbfake ../drivers/fpga.pio)
pico_generate_pio_header(mistusbfake ../drivers/spi.pio)
pico_generate_pio_header(mistusbfake ../drivers/jamma.pio)
pico_generate_pio_header(mistusbfake ../drivers/jammadb9.pio)
pico_enable_stdio_usb(mistusbfake 1)
pico_enable_stdio_uart(mistusbfake 0)
pico_add_extra_outputs(mistusbfake)
example_auto_set_url(mistusbfake)
pico_set_binary_type(mistusbfake no_flash)
target_compile_options(mistusbfake PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  ${archdefs}
  "-DJAMMADB9"
  "-DUSB"
  "-DUSBFAKE"
  "-DTEST_BUILD"
)

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistxilinx
  ${driver_src}
  ${main_src}
  ${picosynth_src}
  )
  # ${usb_src}

target_link_libraries(mistxilinx 
  pico_stdlib 
  mistfirmwarexilinx 
  hardware_spi 
  hardware_pio 
  hardware_flash 
  pico_multicore 
  ${picosynth_lib}
  hardware_i2c)
# target_include_directories(mistxilinx PUBLIC ../usbhost)
target_include_directories(mistxilinx PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistxilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistxilinx ../drivers/spi.pio)
pico_generate_pio_header(mistxilinx ../drivers/jamma.pio)
pico_generate_pio_header(mistxilinx ../drivers/jammadb9.pio)
pico_generate_pio_header(mistxilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistxilinx ../drivers/ps2.pio)
pico_generate_pio_header(mistxilinx ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistxilinx 1)
pico_enable_stdio_uart(mistxilinx 0)
pico_add_extra_outputs(mistxilinx)
example_auto_set_url(mistxilinx)
#pico_set_binary_type(mistusb no_flash)
target_compile_options(mistxilinx PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DXILINX"
  "-DJAMMADB9"
  "-DMIST_ROOT=\"/MIST\""
  "-DBUFFER_FPGA"
  # "-DUSBFAKE"
  # "-DUSB"
  # "-DMIST_USB"
  # "-DUSB_STORAGE"
  #"-DDEVKIT_DEBUG"
  "-DUSB_DEBUG_OFF"
#  "-DALTERA_FPGA"
  "-DPICOSYNTH"
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
pico_generate_pio_header(mistusbxilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/ps2.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusbxilinx 0)
pico_enable_stdio_uart(mistusbxilinx 0)
pico_add_extra_outputs(mistusbxilinx)
example_auto_set_url(mistusbxilinx)
#pico_set_binary_type(mistusb no_flash)
target_compile_options(mistusbxilinx PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DXILINX"
  # "-DMB2"
  "-DJAMMADB9"
  "-DUSB"
  "-DBUFFER_FPGA"
  "-DMIST_USB"
  "-DUSB_STORAGE"
  "-DBOOT_FLASH_ON_ERROR"
  #"-DDEVKIT_DEBUG"
  "-DUSB_DEBUG_OFF"
  "-DMIST_ROOT=\"/MIST\""
  "-DPICOSYNTH"
#  "-DALTERA_FPGA"
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
pico_generate_pio_header(mistusb ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistusb ../drivers/ps2.pio)
pico_generate_pio_header(mistusb ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistusb 0)
pico_enable_stdio_uart(mistusb 0)
pico_add_extra_outputs(mistusb)
example_auto_set_url(mistusb)
# pico_set_binary_type(mistusb no_flash)
target_compile_options(mistusb PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  # "-DXILINX"
  "-DJAMMADB9"
  "-DUSB"
  "-DMIST_USB"
  "-DALTERA_FPGA"
  "-DQMTECH"
  "-DUSB_STORAGE"
  "-DPICOSYNTH"
  "-DUSB_DEBUG_OFF"
#  "-DBUFFER_FPGA"
)

####################################################################################
# RAM build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistmb2xilinxram
  ${driver_mb2_src}
  ${main_src}
  # ${picosynth_src}
  ${usb_src}
  )

#   add_executable(mistram
#   ${driver_src}
#   ${main_src}
#   )

# target_link_libraries(mistram pico_stdlib mistfirmware${testarch} hardware_spi hardware_pio)


target_link_libraries(mistmb2xilinxram 
  pico_stdlib 
  mistfirmwarexilinx 
  # mistfirmwareusbxilinx
  hardware_spi 
  hardware_pio 
  # hardware_flash 
  # ${picosynth_lib}
  hardware_i2c
  pico_multicore)
  # pico_multicore tinyusb_host tinyusb_board)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2xilinxram ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2xilinxram ../drivers/spi.pio)
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
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  # "-DBUFFER_FPGA"
  "-DXILINX"
  "-DMB2"
  "-DIPC_MASTER"
  "-DUSB_DEBUG_OFF"
  "-DBOOT_FLASH_ON_ERROR"
  # "-DPICOSYNTH"
  # "-DMB2USB"

  # USB
  # "-DUSB"
  # "-DMIST_USB"
  # "-DUSB_STORAGE"
  # "-DTEST_BUILD"
)

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistmb2xilinx
  ${driver_mb2_src}
#  ../drivers/debug.c
  ${main_src}
  ${usb_src}
  ${picosynth_src}
  )

# mistfirmwareusbxilinx
target_link_libraries(mistmb2xilinx 
  pico_stdlib
  # mistfirmwarexilinx 
  mistfirmwareusbxilinx
  hardware_spi 
  hardware_pio 
  hardware_flash 
  ${picosynth_lib}
  # pico_multicore)
  pico_multicore tinyusb_host tinyusb_board)
target_include_directories(mistmb2xilinx PUBLIC ../usbhost)
target_include_directories(mistmb2xilinx PUBLIC ../picosynth)
add_compile_options(-O2)
pico_generate_pio_header(mistmb2xilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/spi.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistmb2xilinx ../drivers/uart_tx.pio)
pico_enable_stdio_usb(mistmb2xilinx 0)
# pico_enable_stdio_usb(mistmb2xilinx 1)
pico_enable_stdio_uart(mistmb2xilinx 0)
pico_add_extra_outputs(mistmb2xilinx)
example_auto_set_url(mistmb2xilinx)
set_target_properties(mistmb2xilinx PROPERTIES PICO_TARGET_LINKER_SCRIPT "./standalone.ld")
# pico_set_binary_type(mistmb2xilinx no_flash)
target_compile_options(mistmb2xilinx PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DXILINX"
  "-DUSB"
  "-DMIST_USB"
  "-DUSB_STORAGE"
  "-DMB2"
  "-DBUFFER_FPGA"
  # "-DMB2USB"
  "-DIPC_MASTER"
  "-DUSB_DEBUG_OFF"
  "-DBOOT_FLASH_ON_ERROR"
  "-DPICOSYNTH"
  # "-DPIODEBUG"
)

####################################################################################
# FLASH build - release binary for Altera NeptUno board -with single CPU middleboard.
add_executable(mistusbn1p
  ${driver_src}
  ${main_src}
  ${usb_src}
  )

target_link_libraries(mistusbn1p pico_stdlib mistfirmwareusb hardware_spi hardware_pio hardware_flash hardware_i2c tinyusb_host tinyusb_board)
target_include_directories(mistusbn1p PUBLIC ../usbhost)
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
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DXILINX"
  "-DJAMMADB9"
  "-DUSB"
  "-DMIST_USB"
  "-DALTERA_FPGA"
  "-DUSB_STORAGE"
  "-DUSB_DEBUG_OFF"
)

####################################################################################
# Main build RAM test build for ZXUno
add_executable(mistzx1ram
  ${driver_zx1_src}
  ${mainzx1_src}
  ${picosynth_src}
  )

target_link_libraries(mistzx1ram pico_stdlib mistfirmware${testarch} hardware_spi hardware_pio pico_multicore 
${picosynth_lib})
add_compile_options(-O2)
target_include_directories(mistzx1ram PUBLIC ../picosynth)
#pico_generate_pio_header(mistzx1ram ../drivers/fpga.pio)
pico_generate_pio_header(mistzx1ram ../drivers/spi.pio)
#pico_generate_pio_header(mistzx1ram ../drivers/jamma.pio)
#pico_generate_pio_header(mistzx1ram ../drivers/jammadb9.pio)
pico_generate_pio_header(mistzx1ram ../drivers/ps2.pio)
pico_generate_pio_header(mistzx1ram ../drivers/audio_i2s.pio)
pico_generate_pio_header(mistzx1ram ../drivers/ps2tx.pio)
pico_enable_stdio_usb(mistzx1ram 1)
pico_enable_stdio_uart(mistzx1ram 0)
pico_add_extra_outputs(mistzx1ram)
example_auto_set_url(mistzx1ram)
pico_set_binary_type(mistzx1ram no_flash)
target_compile_options(mistzx1ram PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DZXUNO"
  ${archdefs}
  "-DJAMMADB9"
  "-DNO_PS2_TX"
  "-DNO_LEGACY_MODE"
  "-DZXUNO"
  # "-DPICOSYNTH"
  #"-DTEST_BUILD"
)


