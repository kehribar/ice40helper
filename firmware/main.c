// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdbool.h>
#include "systick_delay.h"
#include "xprintf.h"
#include "bytesutil.h"
#include "ice40prog.h"
#include "usb.h"
#include "spiFlash.h"
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
  hardware_init();

  while(1) 
  {
    usb_poll();

    comHandler_task();

    test_task();
  }
}

// ---------------------------------------------------------------------------
static void test_task()
{
  static uint32_t m_lastExecution = 0;
  if((systick_getCounter1ms() - m_lastExecution) > 999)
  {
    m_lastExecution = systick_getCounter1ms();

    gpio_toggle(GPIOB, GPIO3);
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
  xdev_out(usb_sendChar);

  // ...
  spiFlash_init();
  ice40prog_init();

  // Initialize onboard LED
  rcc_periph_clock_enable(RCC_GPIOB);
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
}
