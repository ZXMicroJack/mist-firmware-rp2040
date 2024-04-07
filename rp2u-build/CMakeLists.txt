cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico)
set(BOARD pico_sdk)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
# include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)


project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_SLAVE -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 -DUSB")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_SLAVE -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

# Build project

#include_directories(../drivers . ../usbhost)
include_directories(../drivers .) # nousb

# fpga program  
add_executable(rp2ur rp2u.c
  ../drivers/ps2.c
  ../drivers/ipc.c
  ../drivers/flash.c
  ../drivers/debug.c 
  ../drivers/fifo.c 
  ../drivers/kbd.c
  ../drivers/cookie.c
  ../drivers/jamma.c
  ../drivers/crc16.c)
  #  ../drivers/crc16.c
  #  ../usbhost/hid_app.c
  #  ../usbhost/joypad.c)
  
target_link_libraries(rp2ur
    pico_stdlib 
    hardware_pio 
    hardware_flash 
    hardware_i2c
    pico_i2c_slave
    pico_multicore)
    #pico_multicore
    #tinyusb_host
    #tinyusb_board)
    
add_compile_options(-O2)
#target_include_directories(rp2ur PUBLIC ../usbhost)
pico_enable_stdio_usb(rp2ur 1)
#pico_enable_stdio_usb(rp2ur 0)
pico_enable_stdio_uart(rp2ur 0)
pico_add_extra_outputs(rp2ur)
example_auto_set_url(rp2ur)
pico_generate_pio_header(rp2ur ../drivers/jamma.pio)
pico_generate_pio_header(rp2ur ../drivers/jammadb9.pio)
pico_generate_pio_header(rp2ur ../drivers/ps2.pio)
pico_generate_pio_header(rp2ur ../drivers/ps2tx.pio)
# target_compile_options(rp2ur PUBLIC "-DIPC_SLAVE" "-DRP2U" "-DUSBDEV")
target_compile_options(rp2ur PUBLIC "-DIPC_SLAVE" "-DRP2U" "-DMATRIX_KEYBOARD")
#target_compile_options(rp2ur PUBLIC "-DIPC_SLAVE" "-DRP2U" "-DMATRIX_KEYBOARD" "-DUSB")

pico_set_binary_type(rp2ur no_flash)
#set_target_properties(rp2u PROPERTIES PICO_TARGET_LINKER_SCRIPT "./standalone.ld")



# fpga program
add_executable(rp2u rp2u.c
  ../drivers/ps2.c
  ../drivers/ipc.c
  ../drivers/flash.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/kbd.c
  ../drivers/cookie.c
  ../drivers/jamma.c
  ../drivers/crc16.c
  ../usbhost/hid_app.c
  ../usbhost/cdc_app.c
  ../usbhost/joypad.c)

target_link_libraries(rp2u
    pico_stdlib
    hardware_pio
    hardware_flash
    hardware_i2c
    pico_i2c_slave
    pico_multicore
    tinyusb_host
    tinyusb_board)

add_compile_options(-O2)
pico_enable_stdio_usb(rp2u 0)
pico_enable_stdio_uart(rp2u 0)
pico_add_extra_outputs(rp2u)
example_auto_set_url(rp2u)
target_include_directories(rp2u PUBLIC ../usbhost)
pico_generate_pio_header(rp2u ../drivers/jamma.pio)
pico_generate_pio_header(rp2u ../drivers/jammadb9.pio)
pico_generate_pio_header(rp2u ../drivers/ps2.pio)
pico_generate_pio_header(rp2u ../drivers/ps2tx.pio)
target_compile_options(rp2u PUBLIC "-DIPC_SLAVE" "-DRP2U" "-DMATRIX_KEYBOARD" "-DUSB")
set_target_properties(rp2u PROPERTIES PICO_TARGET_LINKER_SCRIPT "./standalone.ld")


# fpga program
add_executable(rp2u_nousb rp2u.c
  ../drivers/ps2.c
  ../drivers/ipc.c
  ../drivers/flash.c
  ../drivers/debug.c
  ../drivers/fifo.c
  ../drivers/kbd.c
  ../drivers/cookie.c
  ../drivers/jamma.c
  ../drivers/crc16.c
  ../usbhost/hid_app.c
  ../usbhost/cdc_app.c
  ../usbhost/joypad.c)

target_link_libraries(rp2u_nousb
    pico_stdlib
    hardware_pio
    hardware_flash
    hardware_i2c
    pico_i2c_slave
    pico_multicore)

add_compile_options(-O2)
pico_enable_stdio_usb(rp2u_nousb 1)
pico_enable_stdio_uart(rp2u_nousb 0)
pico_add_extra_outputs(rp2u_nousb)
example_auto_set_url(rp2u_nousb)
pico_generate_pio_header(rp2u_nousb ../drivers/jamma.pio)
pico_generate_pio_header(rp2u_nousb ../drivers/jammadb9.pio)
pico_generate_pio_header(rp2u_nousb ../drivers/ps2.pio)
pico_generate_pio_header(rp2u_nousb ../drivers/ps2tx.pio)
target_compile_options(rp2u_nousb PUBLIC "-DIPC_SLAVE" "-DRP2U" "-DMATRIX_KEYBOARD") # "-DUSB")

set_target_properties(rp2u_nousb PROPERTIES PICO_TARGET_LINKER_SCRIPT "./standalone.ld")




