// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdbool.h>
#include "systick_delay.h"
#include "xprintf.h"
#include "ice40prog.h"
#include "usb.h"
#include "comHandler.h"

// ----------------------------------------------------------------------------
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// ---------------------------------------------------------------------------
static void test_task();
static void hardware_init();

// ---------------------------------------------------------------------------
int main()
{
  static uint8_t localBuf[128];

  hardware_init();

  while(1) 
  {
    usb_poll();

    if(usb_isConnected())
    {
      int32_t datalen = usb_rxDataAmount();
      if(datalen == 128)
      {
        gpio_toggle(GPIOB, GPIO3);
        usb_rxData(localBuf, datalen);
        comHandler_analyse(localBuf, datalen);
      }
    }

    test_task();
  }
}

// ---------------------------------------------------------------------------
static void test_task()
{
  static uint32_t m_lastExecution = 0;
  if((systick_getCounter1ms() - m_lastExecution) > 99)
  {
    m_lastExecution = systick_getCounter1ms();

    // ...
  }
}

// ---------------------------------------------------------------------------
static void hardware_init()
{
  // ...
  rcc_clock_setup_in_hsi_out_48mhz();
  systick_delay_init();

  // ...
  usb_init();
  ice40prog_init();

  // ...
  xdev_out(usb_sendChar);

  // Initialize onboard LED
  rcc_periph_clock_enable(RCC_GPIOB);
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
}
