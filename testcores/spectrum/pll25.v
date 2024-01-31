module pll (
	input wire inclk0,
	output wire c0);

	reg clk = 1'b0;

	always @(posedge inclk0)
    clk <= !clk;

  assign c0 = clk;

endmodule
