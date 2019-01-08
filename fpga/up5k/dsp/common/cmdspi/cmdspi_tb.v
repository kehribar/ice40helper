// ----------------------------------------------------------------------------
// ...
// ----------------------------------------------------------------------------
`timescale 1ns / 1ps
`default_nettype none

// ----------------------------------------------------------------------------
module top_tb();

// ----------------------------------------------------------------------------
reg clk, rst;

// ----------------------------------------------------------------------------
reg CSN;
wire MOSI;
wire MISO;
reg SCLK;
wire we;
reg[31:0] rdat;
wire[31:0] wdat;
wire[6:0] addr;

// ----------------------------------------------------------------------------
integer i;
reg[39:0] txreg_top;

// ----------------------------------------------------------------------------
// Initialize all variables and run the test
initial begin
  $dumpfile("cmdspi_tb.vcd");
  $dumpvars(0, cmdspi_inst);
  clk <= 1;
  rst <= 1;  
  CSN <= 1;
  // MOSI <= 0;
  SCLK <= 1;
  rdat <= 0;
  #50 rst <= 0;
  rdat <= 32'hA5A5A5A5;
  #5 txreg_top <= 40'h0112345678;  
  #5 CSN <= 0;
  for(i=0;i<40;i++) begin
    #10 SCLK <= 0;
    #10 SCLK <= 1;
    txreg_top <= {txreg_top[38:0], 1'b0};
  end
  #5 CSN <= 1;
  rdat <= 32'h5A5A5A5A;  
  #5 txreg_top <= 40'h0112345678;  
  #5 CSN <= 0;
  for(i=0;i<40;i++) begin
    #10 SCLK <= 0;
    #10 SCLK <= 1;
    txreg_top <= {txreg_top[38:0], 1'b0};
  end
  #5 CSN <= 1;
  rdat <= 32'hA5A5A5A5; 
  #5 txreg_top <= 40'h8212345678;
  #50 CSN <= 0;
  for(i=0;i<40;i++) begin       
    #10 SCLK <= 0;
    #10 SCLK <= 1;
    txreg_top <= {txreg_top[38:0], 1'b0};
  end
  #5 CSN <= 1;
  #250 $finish;
end

assign MOSI = txreg_top[39];

// ----------------------------------------------------------------------------
// Log the variables
// always @(posedge clk) begin
  // if(rst == 0) begin
    // $display("%g, %d, %d",$time, rxValid, rxData);
//   end
// end

// ----------------------------------------------------------------------------
// Simulation clock generator
always begin
  #1 clk = ~clk;
end

// --------------------------------------------------------------------------
// DUT
// --------------------------------------------------------------------------  
cmdspi cmdspi_inst(
  .clk(clk),
  .rst(rst),
  .CSN(CSN),
  .SCLK(SCLK),
  .MOSI(MOSI),
  .MISO(MISO),
  .we(we),
  .addr(addr),
  .rdat(rdat),
  .wdat(wdat)
);
 
endmodule