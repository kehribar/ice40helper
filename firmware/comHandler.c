// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include "comHandler.h"
#include <stdbool.h>
#include "spiFlash.h"
#include "ice40prog.h"
#include "bytesutil.h"
#include "usb.h"
#include "ice40data.h"

// ----------------------------------------------------------------------------
#include <libopencm3/stm32/gpio.h>

// ----------------------------------------------------------------------------
static uint8_t m_buff[128];

// ----------------------------------------------------------------------------
void comHandler_init()
{

}

// ----------------------------------------------------------------------------
void comHandler_task()
{
  int32_t datalen = usb_rxDataAmount();
  if(datalen == 128)
  {
    gpio_set(GPIOF, GPIO0);
    usb_rxData(m_buff, datalen);
    comHandler_analyse(m_buff, datalen);
    gpio_clear(GPIOF, GPIO0);
  }
  else if(datalen > 128)
  {
    usb_rxData(m_buff, datalen);
  }
}

// ----------------------------------------------------------------------------
void comHandler_analyse(uint8_t* buf, int32_t len)
{
  uint8_t response[64];
  uint8_t standardResponse = true;

  // ...
  switch(buf[0])
  {
    // ------------------------------------------------------------------------
    // Loopback
    case 0:
    {
      for(int32_t i=0;i<64;i++)
      {
        response[i] = buf[i];
      }
      usb_txData(response, 64);
      standardResponse = false;
      break;
    }
    // ------------------------------------------------------------------------
    // ICE40 programming transmit data
    case 1:
    {
      ice40prog_txData((uint8_t*)(buf+1), (len-1));
      break;
    }
    // ------------------------------------------------------------------------
    // ICE40 programming start
    case 2:
    {
      ice40prog_start();
      break;
    }
    // ------------------------------------------------------------------------
    // ICE40 programming finish
    case 3:
    {
      ice40prog_finish();
      break;
    }
    // ------------------------------------------------------------------------
    // SPI Flash sector erase
    case 4:
    {
      uint32_t packet_ind = buf[1];
      spiFlash_write_enable();
      spiFlash_sectorErase(packet_ind * 0x1000);
      while(spiFlash_isBusy());
      spiFlash_write_disable();
      break;
    }
    // ------------------------------------------------------------------------
    // SPI Flash write initialise
    case 5:
    {
      uint32_t address = make32b(buf,1); 
      spiFlash_write_initalise(address);
      break;
    }
    // ------------------------------------------------------------------------
    // SPI Flash write stream data push
    case 6:
    {
      uint8_t* data = (uint8_t*)(buf + 1);
      spiFlash_write_push(data, 127);
      break;
    }
    // ------------------------------------------------------------------------
    // SPI Flash write finalise
    case 7:
    {
      spiFlash_write_finalise();
      break;
    }
    // ------------------------------------------------------------------------
    // Load bitstream from SPI Flash Memory
    case 8:
    {
      uint32_t address = make32b(buf,1); 
      ice40prog_loadFromSpiFlash(address);
      break;
    }
    // ------------------------------------------------------------------------
    // Basic FPGA register write
    case 9:
    {
      uint8_t address = buf[1]; 
      uint32_t data = make32b(buf,2); 
      ice40data_writeRegister32b(address, data);
      break;
    }
    // ------------------------------------------------------------------------
    // Basic FPGA register read
    case 10:
    {
      uint8_t address = buf[1]; 
      uint32_t data = ice40data_readRegister32b(address); 
      put32b(response, 0, data);
      usb_txData(response, 64);
      standardResponse = false;
      break;
    }
  }

  // ...
  if(standardResponse)
  {
    response[0] = buf[0];
    response[1] = 0x11;
    response[2] = 0x22;
    response[3] = 0x33;
    usb_txData(response, 64);
  }
}
