// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// RX: [0] + [7b address] + [32b data]
// TX:
// ----------------------------------------------------------------------------
// RX: [1] + [7b address] +
// TX:                      [32b data]
// ----------------------------------------------------------------------------
`default_nettype none

// ----------------------------------------------------------------------------
module cmdspi(
  // --------------------------------------------------------------------------
  input clk, // Main clock
  input rst, // Enable high reset
  // --------------------------------------------------------------------------
  input CSN, // Enable low chip select signal
  input SCLK, // Clock signal for slave spi
  input MOSI, // Fpga send, mcu receives
  output MISO, // Fpga receives, mcu sends
  // --------------------------------------------------------------------------
  output we, // Write enable
  output reg [6:0] addr, // Final 7bit r/w address
  input [31:0] rdat, // Final 32bit read databus
  output reg [31:0] wdat // Final 32bit Write databus
  // --------------------------------------------------------------------------
);

// ----------------------------------------------------------------------------
reg we_flag = 0;
reg CSN_reg = 0;
reg MOSI_reg = 0;
reg SCLK_reg = 0;
reg we_flag_d = 0;
reg CSN_reg_d = 0;
reg CSN_reg_dd = 0;
reg MOSI_reg_d = 0;
reg SCLK_reg_d = 0;
reg SCLK_reg_dd = 0;
reg writeEnable = 0;
reg [31:0] rxReg = 0;
reg [31:0] txReg = 0;
reg [5:0] bitCount = 0;
reg addressReceived = 0;
reg txReg_latchFlag = 0;

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
always @(posedge clk) begin
  // ...
  CSN_reg <= CSN;
  CSN_reg_d <= CSN_reg;
  CSN_reg_dd <= CSN_reg_d;

  // ...
  MOSI_reg <= MOSI;
  MOSI_reg_d <= MOSI_reg;

  // ...
  SCLK_reg <= SCLK;
  SCLK_reg_d <= SCLK_reg;
  SCLK_reg_dd <= SCLK_reg_d;
end

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
wire sclk_falling;
assign sclk_falling = (~SCLK_reg_d) & (SCLK_reg_dd);

wire sclk_falling_valid;
assign sclk_falling_valid = sclk_falling & (~CSN_reg_d) & (~CSN_reg_dd);

wire we_assert;
assign we_assert = (bitCount[5] == 1'b1) &
                   (addressReceived == 1'b1) &
                   (writeEnable == 1'b1);

wire csn_falling;
assign csn_falling = (~CSN_reg_d) & (CSN_reg_dd);

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
always @(posedge clk) begin
  if(rst == 1) begin
    bitCount <= 0;
    we_flag_d <= 0;
  end else begin

    if(we_assert) begin
      wdat <= {rxReg[30:0], MOSI_reg_d};
    end

    if(txReg_latchFlag) begin
      txReg_latchFlag <= 1'b0;
      txReg <= rdat;
    end

    we_flag <= we_assert;
    we_flag_d <= we_flag;

    if(csn_falling) begin
      bitCount <= 6'd6;
      rxReg <= {32{1'b0}};
      writeEnable <= 1'b0;
      addressReceived <= 1'b0;
    end

    if(sclk_falling_valid) begin

      txReg <= {txReg[30:0], 1'b0};
      rxReg <= {rxReg[30:0], MOSI_reg_d};

      if(bitCount[5] == 1'b1) begin
        bitCount <= 6'd30;
        if(!addressReceived) begin
          txReg_latchFlag <= 1'b1;
          addressReceived <= 1'b1;
          writeEnable <= !rxReg[6];
          addr <= {rxReg[5:0], MOSI_reg_d};
        end
      end else begin
        bitCount <= bitCount - 1;
      end
    end

  end
end

// ...
assign we = we_flag_d;
assign MISO = txReg[31];

endmodule