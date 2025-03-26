cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
set(PICO_BOARD pico)
set(BOARD pico_sdk)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSBDEV -DPIODEBUG -Wno-error=incompatible-pointer-types")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSBDEV -Wno-error=incompatible-pointer-types")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSBDEV -DCFG_TUSB_DEBUG_PRINTF=uprintf -DCFG_TUSB_DEBUG=3")

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

# Example source
add_executable(host_cdc_msc_hid)

target_sources(host_cdc_msc_hid PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/hid_app_mist.c
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/msc_app.c
        ${CMAKE_CURRENT_LIST_DIR}/joypad.c
        ${CMAKE_CURRENT_LIST_DIR}/cdc_app.c
        ${CMAKE_CURRENT_LIST_DIR}/xinput.c
        ../drivers/ps2.c
        ../drivers/fifo.c
        ../drivers/debug.c
        ../contrib/tusb_xinput/xinput_host.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(host_cdc_msc_hid PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ../drivers
        ../contrib/tusb_xinput)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_host
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(host_cdc_msc_hid PUBLIC pico_stdlib tinyusb_host tinyusb_board hardware_pio)

pico_add_extra_outputs(host_cdc_msc_hid)
pico_enable_stdio_usb(host_cdc_msc_hid 0)
pico_enable_stdio_uart(host_cdc_msc_hid 0)
# pico_set_binary_type(host_cdc_msc_hid no_flash)

pico_generate_pio_header(host_cdc_msc_hid ../drivers/ps2.pio)
pico_generate_pio_header(host_cdc_msc_hid ../drivers/ps2tx.pio)
pico_generate_pio_header(host_cdc_msc_hid ../drivers/uart_tx.pio)

target_link_options(host_cdc_msc_hid PRIVATE -Xlinker --print-memory-usage -g)
target_compile_options(host_cdc_msc_hid PRIVATE -Wall -Wextra -g  
  -DPICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY=PICO_HIGHEST_IRQ_PRIORITY
  -DPIODEBUG
)

# add url via pico_set_program_url
example_auto_set_url(host_cdc_msc_hid)
pico_set_binary_type(host_cdc_msc_hid no_flash)
