// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include "ice40data.h"
#include "systick_delay.h"
#include "atomic.h"
#include "xprintf.h"

// ----------------------------------------------------------------------------
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>

// ----------------------------------------------------------------------------
uint8_t spi_exchange(uint8_t tx)
{
  uint8_t rv;
  spi_send8(SPI1, tx);
  rv = spi_read8(SPI1);
  return rv;
}

// ----------------------------------------------------------------------------
uint32_t ice40data_readRegister32b(uint8_t address)
{
  uint32_t t8;
  uint32_t rv = 0;

  // Lower the chip select
  gpio_clear(GPIOB, GPIO1);
  _delay_nop(10);

  // Send address
  spi_exchange(address + 0x80);
 
  // Collect the received data 
  t8 = spi_exchange(0x00);
  rv = (t8 << 24);

  t8 = spi_exchange(0x00);
  rv |= (t8 << 16);

  t8 = spi_exchange(0x00);
  rv |= (t8 <<  8);

  t8 = spi_exchange(0x00);
  rv |= (t8 <<  0);

  // Set chip select to normal
  _delay_nop(10);
  gpio_set(GPIOB, GPIO1);

  return rv;
}

// ----------------------------------------------------------------------------
void ice40data_writeRegister32b(uint8_t address, uint32_t data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Lower the chip select
    gpio_clear(GPIOB, GPIO1);
    _delay_nop(10);

    // Send address
    spi_exchange(address + 0x00);

    // Send data
    spi_exchange((data & 0xFF000000) >> 24);
    spi_exchange((data & 0x00FF0000) >> 16);
    spi_exchange((data & 0x0000FF00) >>  8);
    spi_exchange((data & 0x000000FF) >>  0);

    // Set chip select to normal
    _delay_nop(10);
    gpio_set(GPIOB, GPIO1);
  }
}
