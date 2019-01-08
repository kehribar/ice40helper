// ----------------------------------------------------------------------------
//
// 
// ----------------------------------------------------------------------------
`default_nettype none

// ----------------------------------------------------------------------------
module smul_16x16 (
  input clk,
  input rst,
  input signed [15:0] a,
  input signed [15:0] b,
  output reg signed [31:0] outp
);

SB_MAC16 SB_MAC16_inst(
  .A(a),
  .B(b),
  .C(16'b0),
  .D(16'b0),
  .CLK(clk),
  .CE(1'b1),
  .IRSTTOP(rst),
  .IRSTBOT(rst),
  .ORSTTOP(rst),
  .ORSTBOT(rst),
  .AHOLD(1'b0),
  .BHOLD(1'b0),
  .CHOLD(1'b0),
  .DHOLD(1'b0),
  .OHOLDTOP(1'b0),
  .OHOLDBOT(1'b0),
  .OLOADTOP(1'b0),
  .OLOADBOT(1'b0),
  .ADDSUBTOP(1'b0),
  .ADDSUBBOT(1'b0),
  .CO(),
  .CI(1'b0),
  .O(outp)
);

 defparam SB_MAC16_inst.B_SIGNED                  = 1'b1;
 defparam SB_MAC16_inst.A_SIGNED                  = 1'b1;
 defparam SB_MAC16_inst.MODE_8x8                  = 1'b0;

 defparam SB_MAC16_inst.BOTADDSUB_CARRYSELECT     = 2'b00;
 defparam SB_MAC16_inst.BOTADDSUB_UPPERINPUT      = 1'b0;
 defparam SB_MAC16_inst.BOTADDSUB_LOWERINPUT      = 2'b00;
 defparam SB_MAC16_inst.BOTOUTPUT_SELECT          = 2'b11;

 defparam SB_MAC16_inst.TOPADDSUB_CARRYSELECT     = 2'b00;
 defparam SB_MAC16_inst.TOPADDSUB_UPPERINPUT      = 1'b0;
 defparam SB_MAC16_inst.TOPADDSUB_LOWERINPUT      = 2'b00;
 defparam SB_MAC16_inst.TOPOUTPUT_SELECT          = 2'b11;

 defparam SB_MAC16_inst.PIPELINE_16x16_MULT_REG2  = 1'b1;
 defparam SB_MAC16_inst.PIPELINE_16x16_MULT_REG1  = 1'b1;
 defparam SB_MAC16_inst.BOT_8x8_MULT_REG          = 1'b1;
 defparam SB_MAC16_inst.TOP_8x8_MULT_REG          = 1'b1;
 defparam SB_MAC16_inst.D_REG                     = 1'b0;
 defparam SB_MAC16_inst.B_REG                     = 1'b1;
 defparam SB_MAC16_inst.A_REG                     = 1'b1;
 defparam SB_MAC16_inst.C_REG                     = 1'b0;

endmodule