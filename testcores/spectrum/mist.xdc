set_property -dict {PACKAGE_PIN V8 IOSTANDARD LVTTL} [get_ports mist_miso]
set_property -dict {PACKAGE_PIN V7 IOSTANDARD LVTTL} [get_ports mist_mosi]
set_property -dict {PACKAGE_PIN W7 IOSTANDARD LVTTL} [get_ports mist_sck]
set_property -dict {PACKAGE_PIN W9 IOSTANDARD LVTTL} [get_ports mist_confdata0]
set_property -dict {PACKAGE_PIN W5 IOSTANDARD LVTTL} [get_ports mist_ss2]
set_property -dict {PACKAGE_PIN W6 IOSTANDARD LVTTL} [get_ports mist_ss3]
set_property -dict {PACKAGE_PIN Y9 IOSTANDARD LVTTL} [get_ports mist_ss4]

#set_property ALLOW_COMBINATORIAL_LOOPS TRUE [get_nets pong_inst/user_io/SPI_MISO_i_13_0]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets mist_sck_IBUF]

#16  v8_miso \   uart0 tx, SPI0RX
#17  w9_mosi |-- sdcard high level / uart0 rx, SPI0CSN
#18  w7_sck  |   SPI0SCK
#19  v7_cs   /   SPI0TX
