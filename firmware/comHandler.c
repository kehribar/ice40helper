// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include "comHandler.h"
#include <stdbool.h>
#include "usb.h"
#include "ice40prog.h"

// ----------------------------------------------------------------------------
void comHandler_init()
{

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
    // ICE40 transmit data
    case 1:
    {
      ice40prog_txData((uint8_t*)(buf+1), (len-1));
      break;
    }
    // ------------------------------------------------------------------------
    // ICE40 start
    case 2:
    {
      ice40prog_start();
      break;
    }
    // ------------------------------------------------------------------------
    // ICE40 finish
    case 3:
    {
      ice40prog_finish();
      break;
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
