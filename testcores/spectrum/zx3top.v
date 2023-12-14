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
  input wire mist_confdata0,
  input wire mist_ss2,
  input wire mist_ss3,
  input wire mist_ss4,
  output wire sdram_clk,
  output wire sdram_cke,
  output wire sdram_dqmh_n,
  output wire sdram_dqml_n,
  output wire sdram_cas_n,
  output wire sdram_ras_n,
  output wire sdram_we_n,
  output wire sdram_cs_n,
  output wire[1:0] sdram_ba,
  output wire[12:0] sdram_addr,
  inout wire[15:0] sdram_dq,
  output wire testled,

  // forward JAMMA DB9 data
  output wire joy_clk,
  input wire xjoy_clk,
  output wire joy_load_n,
  input wire xjoy_load_n,
  input wire joy_data,
  output wire xjoy_data


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
   .CLOCK_27({clk50mhz, clk50mhz}),
   .SPI_DO(mist_miso),
   .SPI_DI(mist_mosi),
   .SPI_SCK(mist_sck),
   .CONF_DATA0(mist_confdata0),
   .SPI_SS2(mist_ss2),
   .SPI_SS3(mist_ss3),
	 .SPI_SS4(mist_ss4),
   .VGA_HS(vga_hs),
   .VGA_VS(vga_vs),
   .VGA_R(vga_r[7:2]),
   .VGA_G(vga_g[7:2]),
   .VGA_B(vga_b[7:2]),
   .LED(testled),
   .SDRAM_A(sdram_addr), //std_logic_vector(12 downto 0)
   .SDRAM_DQ(sdram_dq),  // std_logic_vector(15 downto 0);
   .SDRAM_DQML(sdram_dqml_n), // out
   .SDRAM_DQMH(sdram_dqmh_n), // out
   .SDRAM_nWE(sdram_we_n), //	:  out 		std_logic;
   .SDRAM_nCAS(sdram_cas_n), //	:  out 		std_logic;
   .SDRAM_nRAS(sdram_ras_n), //	:  out 		std_logic;
   .SDRAM_nCS(sdram_cs_n), //	:  out 		std_logic;
   .SDRAM_BA(sdram_ba), //		:  out 		std_logic_vector(1 downto 0);
   .SDRAM_CLK(sdram_clk), //	:  out 		std_logic;
   .SDRAM_CKE(sdram_cke) //	:  out 		std_logic;
);

// JAMMA interface
assign joy_clk = xjoy_clk;
assign joy_load_n = xjoy_load_n;
assign xjoy_data = joy_data;

  //output wire joy_clk,
  //input wire xjoy_clk,
  //output wire joy_load_n,
  //input wire xjoy_load_n,
  //input wire joy_data,
  //output wire xjoy_data

//joydecoder joydecoder_inst (
  //.clk(clk50mhz),
  //.joy_data(joy_data),
  //.joy_latch_megadrive(1'b1),
  //.joy_clk(joy_clk),
  //.joy_load_n(joy_load_n)
  //);


//entity spectrum_mist is
//generic (
//	-- Model to generate
//	-- 0 = 48 K
//	-- 1 = 128 K
//	-- 2 = +2A/+3
//	MODEL				:	integer := 1
//);

//port (
//	-- Clocks
//	CLOCK_27	:	in	std_logic_vector(1 downto 0);

//	-- LED
//	LED		:	out	std_logic;

//	-- VGA
//	VGA_R		:	out	std_logic_vector(5 downto 0);
//	VGA_G		:	out	std_logic_vector(5 downto 0);
//	VGA_B		:	out	std_logic_vector(5 downto 0);
//	VGA_HS		:	out	std_logic;
//	VGA_VS		:	out	std_logic;

//	-- Serial
//--	UART_RXD	:	in	std_logic;
//--	UART_TXD	:	out	std_logic;

//	-- SDRAM
//	SDRAM_A		:	out		std_logic_vector(12 downto 0);
//	SDRAM_DQ		:	inout		std_logic_vector(15 downto 0);
//	SDRAM_DQML	:  out 		std_logic;
//	SDRAM_DQMH	:  out 		std_logic;
//	SDRAM_nWE	:  out 		std_logic;
//	SDRAM_nCAS	:  out 		std_logic;
//	SDRAM_nRAS	:  out 		std_logic;
//	SDRAM_nCS	:  out 		std_logic;
//	SDRAM_BA		:  out 		std_logic_vector(1 downto 0);
//	SDRAM_CLK	:  out 		std_logic;
//	SDRAM_CKE	:  out 		std_logic;

//   -- AUDIO
//   AUDIO_L         : out std_logic;
//   AUDIO_R         : out std_logic;

//   -- SPI interface to io controller
//   SPI_SCK         : in std_logic;
//   SPI_DO          : inout std_logic;
//   SPI_DI          : in std_logic;
//   SPI_SS2         : in std_logic;
//   SPI_SS3         : in std_logic;
//	SPI_SS4         : in std_logic;
//   CONF_DATA0      : in std_logic
//);




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

