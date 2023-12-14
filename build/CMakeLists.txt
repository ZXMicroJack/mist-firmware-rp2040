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
  #../mist-firmware/main.c
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
  #"-Dspi_init=mister_spi_init"
)

# fpga program
add_executable(mist
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
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  mmc.c
  keyboard.c
  )

target_link_libraries(mist pico_stdlib mistfirmware hardware_spi hardware_pio hardware_flash hardware_i2c)
add_compile_options(-O2)
pico_generate_pio_header(mist ../drivers/fpga.pio)
pico_generate_pio_header(mist ../drivers/spi.pio)
pico_enable_stdio_usb(mist 1)
pico_enable_stdio_uart(mist 0)
pico_add_extra_outputs(mist)
example_auto_set_url(mist)
pico_set_binary_type(mist no_flash)
target_compile_options(mist PUBLIC
  "-DVDATE=\"\""
  "-Dhexdump=driver_hexdump"
  "-DPS2HOST"
)


#target_compile_options(mist PUBLIC
  #"-DVDATE=\"\""
  #)


