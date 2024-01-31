//set_property -dict {PACKAGE_PIN V8 IOSTANDARD LVTTL} [get_ports mist_miso]
//set_property -dict {PACKAGE_PIN V7 IOSTANDARD LVTTL} [get_ports mist_mosi]
//set_property -dict {PACKAGE_PIN W7 IOSTANDARD LVTTL} [get_ports mist_sck]
//set_property -dict {PACKAGE_PIN W9 IOSTANDARD LVTTL} [get_ports mist_confdata0]



module neptuno2_top(
  input wire CLOCK_50,

  output wire [7:0] VGA_R,
  output wire [7:0] VGA_G,
  output wire [7:0] VGA_B,
  output wire VGA_HS,
  output wire VGA_VS,

  inout wire SPI_DO,
  input wire SPI_DI,
  input wire SPI_SCK,
  input wire CONF_DATA0,
  input wire SPI_SS2,
  input wire SPI_SS3,
  input wire SPI_SS4,
  output wire SDRAM_CLK,
  output wire SDRAM_CKE,
  output wire SDRAM_DQMH,
  output wire SDRAM_DQML,
  output wire SDRAM_nCAS,
  output wire SDRAM_nRAS,
  output wire SDRAM_nWE,
  output wire SDRAM_nCS,
  output wire[1:0] SDRAM_BA,
  output wire[12:0] SDRAM_A,
  inout wire[15:0] SDRAM_DQ,
  output wire I2SL,
  output wire I2SC,
  output wire I2SD,
  output wire AUDIO_L,
  output wire AUDIO_R,

  output wire LED,

  input wire EAR,

  // forward JAMMA DB9 data
  output wire JOY_CLK,
  input wire XJOY_CLK,
  output wire JOY_LOAD_N,
  input wire XJOY_LOAD_N,
  input wire JOY_DATA,
  output wire XJOY_DATA


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

spectrum_mist spectrum_mist_inst(
   .CLOCK_27(CLOCK_50),
   .SPI_DO(SPI_DO),
   .SPI_DI(SPI_DI),
   .SPI_SCK(SPI_SCK),
   .CONF_DATA0(CONF_DATA0),
   .SPI_SS2(SPI_SS2),
   .SPI_SS3(SPI_SS3),
	 .SPI_SS4(SPI_SS4),
   .VGA_HS(VGA_HS),
   .VGA_VS(VGA_VS),
   .VGA_R(VGA_R[7:2]),
   .VGA_G(VGA_G[7:2]),
   .VGA_B(VGA_B[7:2]),
   .LED(LED),
   .SDRAM_A(SDRAM_A), //std_logic_vector(12 downto 0)
   .SDRAM_DQ(SDRAM_DQ),  // std_logic_vector(15 downto 0);
   .SDRAM_DQML(SDRAM_DQML), // out
   .SDRAM_DQMH(SDRAM_DQMH), // out
   .SDRAM_nWE(SDRAM_nWE), //	:  out 		std_logic;
   .SDRAM_nCAS(SDRAM_nCAS), //	:  out 		std_logic;
   .SDRAM_nRAS(SDRAM_nRAS), //	:  out 		std_logic;
   .SDRAM_nCS(SDRAM_nCS), //	:  out 		std_logic;
   .SDRAM_BA(SDRAM_BA), //		:  out 		std_logic_vector(1 downto 0);
   .SDRAM_CLK(SDRAM_CLK), //	:  out 		std_logic;
   .SDRAM_CKE(SDRAM_CKE), //	:  out 		std_logic;
   .AUDIO_L(AUDIO_L),
   .AUDIO_R(AUDIO_R),
   //.I2S_BCK(I2SC),
   //.I2S_LRCK(I2SL),
   //.I2S_DATA(I2SD),
   //.AUDIO_IN(EAR),
   //.UART_RX(1'b1),
   //.UART_TX()
);

// JAMMA interface
assign JOY_CLK = XJOY_CLK;
assign JOY_LOAD_N = XJOY_LOAD_N;
assign XJOY_DATA = JOY_DATA;


endmodule

