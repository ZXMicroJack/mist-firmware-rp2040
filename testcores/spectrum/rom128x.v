/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
`define ROM_SIZE 32768

module rom128(output reg[7:0] q, input wire[14:0] address, input wire clock);

   reg [7:0] mem [0:`ROM_SIZE-1] /* synthesis ramstyle = "M144K" */;
   initial begin
     $readmemh("128x.hex", mem);
   end

   always @(posedge clock) begin
     q <= mem[address];
   end
endmodule
