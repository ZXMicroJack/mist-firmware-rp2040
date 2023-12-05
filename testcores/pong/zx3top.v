//set_property -dict {PACKAGE_PIN V8 IOSTANDARD LVTTL} [get_ports mist_miso]
//set_property -dict {PACKAGE_PIN V7 IOSTANDARD LVTTL} [get_ports mist_mosi]
//set_property -dict {PACKAGE_PIN W7 IOSTANDARD LVTTL} [get_ports mist_sck]
//set_property -dict {PACKAGE_PIN W9 IOSTANDARD LVTTL} [get_ports mist_confdata0]


module zx3top(
  input wire clk50mhz,

  output wire [7:0] vga_r,
  output wire [7:0] vga_g,
  output wire [7:0] vga_b,
  output wire vga_hs,
  output wire vga_vs,

  output wire mist_miso,
  input wire mist_mosi,
  input wire mist_sck,
  input wire mist_confdata0
  //,
  //input wire ear,
  //inout wire clkps2,
  //inout wire dataps2,
  //inout wire mouseclk,
  //inout wire mousedata,
  //output wire audio_out_left,
  //output wire audio_out_right,

  //output wire [19:0] sram_addr,
  //inout wire [15:0] sram_data,
  //output wire sram_we_n,
  //output wire sram_oe_n,
  //output wire sram_ub_n,
  //output wire sram_lb_n,

  //output wire flash_cs_n,
  //output wire flash_clk,
  //output wire flash_mosi,
  //input wire flash_miso,
  //output wire flash_wp,
  //output wire flash_hold,

  //output wire uart_tx,
  //input wire uart_rx,
  //output wire uart_rts,
  //output wire uart_reset,
  //output wire uart_gpio0,

  //output wire i2c_scl,
  //inout wire i2c_sda,

  //output wire midi_out,
  //input wire midi_clkbd,
  //input wire midi_wsbd,
  //input wire midi_dabd,

  //input wire joy_data,
  //output wire joy_clk,
  //output wire joy_load_n,

  //input wire xjoy_data,
  //output wire xjoy_clk,
  //output wire xjoy_load_n,

  //output wire i2s_bclk,
  //output wire i2s_lrclk,
  //output wire i2s_dout,

  //output wire sd_cs_n,
  //output wire sd_clk,
  //output wire sd_mosi,
  //input wire sd_miso,

  //output wire dp_tx_lane_p,
  //output wire dp_tx_lane_n,
  //input wire  dp_refclk_p,
  //input wire  dp_refclk_n,
  //input wire  dp_tx_hp_detect,
  //inout wire  dp_tx_auxch_tx_p,
  //inout wire  dp_tx_auxch_tx_n,
  //inout wire  dp_tx_auxch_rx_p,
  //inout wire  dp_tx_auxch_rx_n,

  //output wire testled,   // nos servirá como testigo de uso de la SPI
  //output wire testled2,

  //output wire mb_uart_tx,
  //input wire mb_uart_rx
);

  pong pong_inst(
    .CLOCK_27({clk50mhz, clk50mhz}),
    .SDRAM_nCS(),
    .SPI_DO(mist_miso),
    .SPI_DI(mist_mosi),
    .SPI_SCK(mist_sck),
    .CONF_DATA0(mist_confdata0),
    .VGA_HS(vga_hs),
    .VGA_VS(vga_vs),
    .VGA_R(vga_r[7:2]),
    .VGA_G(vga_g[7:2]),
    .VGA_B(vga_b[7:2])
  );


endmodule


//module pong (
   //input [1:0] CLOCK_27,
   //output 		SDRAM_nCS,

   //// spi interface to mists io processor
	//output      SPI_DO,
	//input       SPI_DI,
	//input       SPI_SCK,
	//input       CONF_DATA0,

   //output reg	VGA_HS,
   //output reg 	VGA_VS,
   //output [5:0] VGA_R,
   //output [5:0] VGA_G,
   //output [5:0] VGA_B
//);

