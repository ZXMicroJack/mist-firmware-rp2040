cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
# include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_SLAVE -DIPC_MASTER -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

# Build project
include_directories(. ../mist-firmware ../mist-firmware/usb ..)

set(driver_src
  ../drivers/fpga.c
  ../drivers/pio_spi.c
  ../drivers/ps2.c
  ../drivers/sdcard.c
  ../drivers/bitfile.c
  ../drivers/ipc.c
  ../drivers/flash.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/kbd.c
  ../drivers/jamma.c
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

set(usb_src
  ../usbhost/hid_app_mist.c
)

# mist library
add_library(mistfirmware STATIC
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
#target_link_libraries(mistfirmware pico_stdlib hardware_spi hardware_flash pico_audio_i2s pico_multicore pico_stdlib)
target_compile_options(mistfirmware PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  # comment for USB here
  #"-DUSB"
  #"-Dspi_init=mister_spi_init"
)

# mist USB library
add_library(mistfirmwareusb STATIC
  ../mist-firmware/usb/hidparser.c
  ../mist-firmware/usb/hid.c
  ../mist-firmware/usb/joystick.c
  ../mist-firmware/usb/timer.c
  ../mist-firmware/usb/xboxusb.c
  ../mist-firmware/usb/usbdebug.c
  ../mist-firmware/usb/pl2303.c
             )

target_compile_options(mistfirmwareusb PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
)

####################################################################################
# Main build RAM test build for functionality Xilinx ZXTres
add_executable(mistram
  ${driver_src}
  ${main_src}
  )

target_link_libraries(mistram pico_stdlib mistfirmware hardware_spi hardware_pio hardware_flash hardware_i2c)
add_compile_options(-O2)
pico_generate_pio_header(mistram ../drivers/fpga.pio)
pico_generate_pio_header(mistram ../drivers/spi.pio)
pico_generate_pio_header(mistram ../drivers/jamma.pio)
pico_generate_pio_header(mistram ../drivers/jammadb9.pio)
pico_enable_stdio_usb(mistram 1)
pico_enable_stdio_uart(mistram 0)
pico_add_extra_outputs(mistram)
example_auto_set_url(mistram)
pico_set_binary_type(mistram no_flash)
target_compile_options(mistram PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DXILINX"
  "-DJAMMADB9"
)


####################################################################################
# RAM build - for testing USB interface to USB host module.
add_executable(mistusbfake
  ${driver_src}
  ${main_src}
  )

target_link_libraries(mistusbfake pico_stdlib mistfirmware mistfirmwareusb hardware_spi hardware_pio hardware_flash hardware_i2c)
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
  "-DXILINX"
  "-DJAMMADB9"
  "-DUSB"
  "-DUSBFAKE"
#  "-DALTERA_FPGA"
)

####################################################################################
# FLASH build - release binary for Xilinx ZXTres board -with single CPU middleboard.
add_executable(mistusbxilinx
  ${driver_src}
  ${main_src}
  ${usb_src}
  )

target_link_libraries(mistusbxilinx pico_stdlib mistfirmware mistfirmwareusb hardware_spi hardware_pio hardware_flash hardware_i2c tinyusb_host tinyusb_board)
target_include_directories(mistusbxilinx PUBLIC ../usbhost)
add_compile_options(-O2)
pico_generate_pio_header(mistusbxilinx ../drivers/fpga.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/spi.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/jamma.pio)
pico_generate_pio_header(mistusbxilinx ../drivers/jammadb9.pio)
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
  "-DJAMMADB9"
  "-DUSB"
  "-DMIST_USB"
#  "-DALTERA_FPGA"
)

####################################################################################
# FLASH build - release binary for Altera NeptUno board -with single CPU middleboard.
add_executable(mistusb
  ${driver_src}
  ${main_src}
  ${usb_src}
  )

target_link_libraries(mistusb pico_stdlib mistfirmware mistfirmwareusb hardware_spi hardware_pio hardware_flash hardware_i2c tinyusb_host tinyusb_board)
target_include_directories(mistusb PUBLIC ../usbhost)
add_compile_options(-O2)
pico_generate_pio_header(mistusb ../drivers/fpga.pio)
pico_generate_pio_header(mistusb ../drivers/spi.pio)
pico_generate_pio_header(mistusb ../drivers/jamma.pio)
pico_generate_pio_header(mistusb ../drivers/jammadb9.pio)
pico_enable_stdio_usb(mistusb 0)
pico_enable_stdio_uart(mistusb 0)
pico_add_extra_outputs(mistusb)
example_auto_set_url(mistusb)
#pico_set_binary_type(mistusb no_flash)
target_compile_options(mistusb PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
  "-DXILINX"
  "-DJAMMADB9"
  "-DUSB"
  "-DMIST_USB"
  "-DALTERA_FPGA"
)

