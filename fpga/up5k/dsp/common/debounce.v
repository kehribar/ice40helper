// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// The circuit continuously clocks the button’s logic level into FF1 and 
// subsequently into FF2. So, FF1 and FF2 always store the last two logic 
// levels of the button. When these two values remain identical for a 
// specified time, then FF3 is enabled, and the stable value is clocked 
// through to the result output.
//
// The XOR gate and N-bit counter accomplish the timing. If the button’s 
// level changes, the values of FF1 and FF2 differ for a clock cycle, 
// clearing the N-bit counter via the XOR gate. If the button’s level is 
// unchanging (i.e. if FF1 and FF2 are the same logic level), then the 
// XOR gate releases the counter’s synchronous clear, and the counter 
// begins to count. The counter continues to increment in this manner 
// until it (1) reaches the specified time and enables the output register 
// or (2) is interrupted and cleared by the XOR gate because the button’s 
// logic level is not yet stable.
//
// The counter’s size determines the time required to validate the button’s 
// stability. When the counter increments to the point that its carry out 
// bit is asserted, it disables itself from incrementing further and enables
// the output register FF3. The circuit remains in this state until a 
// different button value is clocked into FF1, clearing the counter via
// the XOR gate.
// --------------------------------------------------------------------------
// Taken from: https://eewiki.net/pages/viewpage.action?pageId=13599139
// --------------------------------------------------------------------------
`default_nettype none

// --------------------------------------------------------------------------
module debounce
 #(
  parameter N = 1
)(
  input clk, 
  input rst, 
  input inp,
  output reg outp
);

// --------------------------------------------------------------------------
reg [N-1:0] q_reg;
reg [N-1:0] q_next;
reg DFF1, DFF2;
wire q_add;
wire q_reset;

// --------------------------------------------------------------------------
// Xor input flip flops to look for level chage to reset counter
assign q_reset = (DFF1  ^ DFF2);

// --------------------------------------------------------------------------
// Add to counter when q_reg msb is equal to 0
assign q_add = ~(q_reg[N-1]);      

// --------------------------------------------------------------------------
// Combo counter to manage q_next 
always @(q_reset, q_add, q_reg) begin
  case({q_reset , q_add})
    2'b00:
      q_next = q_reg;
    2'b01:
      q_next = q_reg + 1;
    default:
      q_next = { N {1'b0} };
  endcase   
end

// --------------------------------------------------------------------------
// Flip flop inputs and q_reg update
always @ (posedge clk) begin
  if(rst == 1'b1) begin
    DFF1 <= 1'b0;
    DFF2 <= 1'b0;
    q_reg <= { N {1'b0} };
  end else begin    
    DFF1 <= inp;
    DFF2 <= DFF1;
    q_reg <= q_next;
  end
end

// --------------------------------------------------------------------------
// Counter control
always @(posedge clk) begin
  if(q_reg[N-1] == 1'b1) begin
    outp <= DFF2;
  end else begin    
    outp <= outp;
  end
end

endmodule
