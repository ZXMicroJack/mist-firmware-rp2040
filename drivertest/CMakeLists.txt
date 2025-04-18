cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
# include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPC_SLAVE -DIPC_MASTER -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 -DPS2HOST -DJAMMADB9 -DALTERA_FPGA -DQMTECH -DTESTBUILD") 
# -DPIODEBUG 
# -DPIODEBUG")
#-DIPCDEV")
# -DALTERA_FPGA_DEV
set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

# Build project

include_directories(../drivers . ../zlib)

# fpga program  
add_executable(drivertest drivertest.c ../drivers/fpga.c
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
  ../drivers/jtag.c
  ../drivers/crc16.c
  ../drivers/bitstore.c
  ../drivers/ps2detect.c
  ../drivers/gpioirq.c
  # ../zlib/inflate.c
  # ../zlib/deflate.c
  # ../zlib/adler32.c
  # ../zlib/inffast.c
  # ../zlib/inftrees.c
  # ../zlib/crc32.c
  # ../zlib/deflate.c
  # ../zlib/trees.c
  # ../zlib/zutil.c
)

target_link_libraries(drivertest pico_stdlib hardware_spi hardware_pio hardware_flash hardware_i2c pico_i2c_slave)
add_compile_options(-O2)
pico_generate_pio_header(drivertest ../drivers/fpga.pio)
pico_generate_pio_header(drivertest ../drivers/spi.pio)
pico_generate_pio_header(drivertest ../drivers/jamma.pio)
pico_generate_pio_header(drivertest ../drivers/jammaj.pio)
pico_generate_pio_header(drivertest ../drivers/jammadb9.pio)
pico_generate_pio_header(drivertest ../drivers/uart_tx.pio)
pico_generate_pio_header(drivertest ../drivers/ps2.pio)
pico_generate_pio_header(drivertest ../drivers/ps2tx.pio)
pico_enable_stdio_usb(drivertest 1)
pico_enable_stdio_uart(drivertest 0)

target_compile_options(drivertest PUBLIC
  # "-DNO_PS2_TX"
  # "-DNO_LEGACY_MODE"
  # "-DZXUNO"
)


pico_add_extra_outputs(drivertest)
example_auto_set_url(drivertest)
pico_set_binary_type(drivertest no_flash)


# sine wave over i2s test program
add_executable(sine_wave sine_wave.c ../drivers/audio_i2s.c)
target_link_libraries(sine_wave pico_stdlib hardware_spi hardware_pio pico_audio hardware_dma)
pico_generate_pio_header(sine_wave ../drivers/audio_not_i2s.pio)
pico_enable_stdio_usb(sine_wave 1)
pico_enable_stdio_uart(sine_wave 0)
pico_add_extra_outputs(sine_wave)
example_auto_set_url(sine_wave)
pico_set_binary_type(sine_wave no_flash)
target_compile_options(sine_wave PUBLIC
  "-DSAMPLE_RATE=22050"
  "-DMIDI_NOT_I2S"
  "-DSAMPLE_LEN=512"
  "-DPICO_AUDIO_I2S_DATA_PIN=4"
  "-DPICO_AUDIO_I2S_CLOCK_PIN_BASE=2"
  "-DPICO_AUDIO_I2S_PIO=1"
)
