// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
module top
(
  output LED0,
  output LED1, 
  output LED2
);

// ...
wire clk;
SB_HFOSC osc(
  .CLKHFPU(1'b1),
  .CLKHFEN(1'b1),
  .CLKHF(clk)
);

// ...
reg [25:0] led_counter;

// ...
always @(posedge clk) begin
  led_counter <= led_counter + 1;
  LED0 <= led_counter[24];
  LED1 <= ~led_counter[25];
  LED2 <= led_counter[23];
end

endmodule
