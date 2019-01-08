// ----------------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------------
`default_nettype none

// ----------------------------------------------------------------------------
module top
(
  output LED0,
  output LED1,
  output LED2,
  output MISO,
  input MOSI,
  input SCLK,
  input CSN
);

// Internal 48MHz clock
wire clk48;
SB_HFOSC osc(
  .CLKHFPU(1'b1),
  .CLKHFEN(1'b1),
  .CLKHF(clk48)
);

// Generate 32MHz from 48MHz
wire clk;
wire pll_lock;
SB_PLL40_CORE #(
  .FEEDBACK_PATH("SIMPLE"),
  .DIVR(4'b0010),
  .DIVF(7'b0111111),
  .DIVQ(3'b101),
  .FILTER_RANGE(3'b001)
) 
pll_inst(
  .RESETB(1'b1),
  .BYPASS(1'b0),
  .REFERENCECLK(clk48),
  .PLLOUTCORE(clk),
  .LOCK(pll_lock)
);

// Generate reset signal from pll lock
wire rst;
assign rst = ~pll_lock;

// ...
reg [23:0] led_counter;
always @(posedge clk) begin
  led_counter <= led_counter + 1;
  LED2 <= led_counter[23];
  LED1 <= led_counter[22];
  LED0 <= led_counter[21];
end

// --------------------------------------------------------------------------
// SPI based data interface between FPGA & microcontroller
// --------------------------------------------------------------------------
wire we; // Write enable
wire [6:0] addr; // 7bit r/w address
wire [31:0] rdat; // 32bit read databus
wire [31:0] wdat; // 32bit Write databus
cmdspi cmd_inst(
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

// Test register
reg [31:0] tmp32;

// ...
reg signed [15:0] inp0;
reg signed [15:0] inp1;
wire signed [31:0] outp;
smul_16x16 smul_16x16_inst(
  .clk(clk),
  .rst(rst),
  .a(inp0),
  .b(inp1),
  .outp(outp)
);

// ----------------------------------------------------------------------------
// Microcontroller reads from FPGA
// ----------------------------------------------------------------------------  
always @(*) begin    
  case(addr)
    // ------------------------------------------------------------------------
    // 
    // ------------------------------------------------------------------------
    7'h00: begin
      rdat = tmp32;
    end
    // ------------------------------------------------------------------------
    7'h01: begin
      rdat = outp;
    end
    // ------------------------------------------------------------------------
    // 
    // ------------------------------------------------------------------------
    default: begin
      rdat = 32'hDEADC0DE;
    end
  endcase
end

// ----------------------------------------------------------------------------
// Microcontroller writes to FPGA
// ----------------------------------------------------------------------------  
always @(posedge clk) begin
  if(rst) begin
    tmp32 <= 32'h12345678;
  end else begin
    if(we) begin
      case(addr)
        // --------------------------------------------------------------------
        // 
        // --------------------------------------------------------------------
        7'h00: begin
          inp0 <= wdat[15:0];   
        end 
        // --------------------------------------------------------------------
        7'h01: begin
          inp1 <= wdat[15:0];   
        end 
        // --------------------------------------------------------------------
        7'h0A: begin
          tmp32 <= wdat;   
        end         
        // --------------------------------------------------------------------
        // 
        // --------------------------------------------------------------------
        default: begin
          tmp32 <= wdat;
        end
      endcase
    end
  end
end

endmodule
