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
#include <libopencm3/stm32/syscfg.h>

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

    // ...
  }
}

// ---------------------------------------------------------------------------
static void hardware_init()
{
  // ...
  rcc_periph_clock_enable(RCC_GPIOF);
  gpio_mode_setup(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  gpio_set(GPIOF, GPIO0);

  // ...
  rcc_clock_setup_in_hsi48_out_48mhz();
  rcc_periph_clock_enable(RCC_SYSCFG_COMP);
  SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
  rcc_set_usbclk_source(RCC_HSI48);

  // ...
  systick_delay_init();

  // ...
  usb_init();
  xdev_out(usb_sendChar);

  // ...
  spiFlash_init();
  ice40prog_init();

  // ...
  gpio_clear(GPIOF, GPIO0);
}
