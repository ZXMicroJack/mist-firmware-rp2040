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
  # Add these for USB
  #../mist-firmware/usb/hidparser.c
  #../mist-firmware/usb/hid.c
  #../mist-firmware/usb/joystick.c
  #../mist-firmware/usb/timer.c
  #../mist-firmware/usb/xboxusb.c
  #../mist-firmware/usb/usbdebug.c
  #../mist-firmware/usb/pl2303.c
             )
#target_link_libraries(mistfirmware pico_stdlib hardware_spi hardware_flash pico_audio_i2s pico_multicore pico_stdlib)
target_compile_options(mistfirmware PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  # comment for USB here
  #"-DUSB"
  #"-Dspi_init=mister_spi_init"
)

# mist library
add_library(mistfirmwareusb STATIC
  # Add these for USB
  ../mist-firmware/usb/hidparser.c
  ../mist-firmware/usb/hid.c
  ../mist-firmware/usb/joystick.c
  ../mist-firmware/usb/timer.c
  ../mist-firmware/usb/xboxusb.c
  ../mist-firmware/usb/usbdebug.c
  ../mist-firmware/usb/pl2303.c
             )

#target_link_libraries(mistfirmware pico_stdlib hardware_spi hardware_flash pico_audio_i2s pico_multicore pico_stdlib)
target_compile_options(mistfirmware PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  # comment for USB here
  #"-DUSB"
  #"-Dspi_init=mister_spi_init"
)

target_compile_options(mistfirmwareusb PUBLIC
  "-DVDATE=\"\""
  "-DMIST"
  # comment for USB here
  #"-DUSB"
  #"-Dspi_init=mister_spi_init"
)

# fpga program
add_executable(mistram
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


# fpga program
add_executable(mistusbfake
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
)

add_executable(mistusb
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
  glue.c
  spi.c
  usbdev.c
  hardware.c
  mist.c
  mistmain.c
  mmc.c
  keyboard.c
  fpgaconfig.c
  ../usbhost/hid_app_mist.c
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
)




#target_compile_options(mist PUBLIC
  #"-DVDATE=\"\""
  #)


