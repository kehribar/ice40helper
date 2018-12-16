// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
module top
(
  input clk,
  output led0,
  output led1
);

// ..
reg [25:0] led_counter;

// ...
always @(posedge clk) begin
  led_counter <= led_counter + 1;
  led0 <= led_counter[24];
  led1 <= ~led_counter[25];
end

endmodule
