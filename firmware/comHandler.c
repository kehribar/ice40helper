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
  if(usb_isConnected())
  {
    int32_t datalen = usb_rxDataAmount();
    if(datalen == 128)
    {
      gpio_toggle(GPIOB, GPIO3);
      usb_rxData(m_buff, datalen);
      comHandler_analyse(m_buff, datalen);
    }
  }
}

// ----------------------------------------------------------------------------
void comHandler_analyse(uint8_t* buf, int32_t len)
{
  uint8_t standardResponse = true;

  // ...
  switch(buf[0])
  {
    // ------------------------------------------------------------------------
    // Test
    case 0:
    {
      // ...
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
    // SPI Flash write initialise
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
    }
  }

  // ...
  if(standardResponse)
  {
    uint8_t response[64];
    response[0] = 0xDE;
    response[1] = 0xAD;
    response[2] = 0xC0;
    response[3] = 0xDE;
    usb_txData(response, 64);
  }
}
