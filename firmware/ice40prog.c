// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <stdbool.h>
#include "ice40prog.h"
#include "systick_delay.h"
#include "spiFlash.h"
#include "bytesutil.h"
#include "xprintf.h"

// ----------------------------------------------------------------------------
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>

// ----------------------------------------------------------------------------
static void ice40prog_setCSHigh(uint8_t isHigh)
{
  if(isHigh)
  {
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);
    gpio_set(GPIOB, GPIO1);
  }
  else
  {
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);
    gpio_clear(GPIOB, GPIO1);
  }
}

// ----------------------------------------------------------------------------
static void ice40prog_fpgaResetHigh(uint8_t isHigh)
{
  if(isHigh)
  {
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO0);
  }
  else
  {
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO0);
    gpio_clear(GPIOA, GPIO0);
  }
}

// ----------------------------------------------------------------------------
static uint8_t ice40prog_cdoneCheck()
{
  return gpio_get(GPIOF, GPIO1);
}

// ----------------------------------------------------------------------------
static void ice40prog_spiSend(uint8_t ch)
{
  spi_send8(SPI1, ch);
}

// ----------------------------------------------------------------------------
static void ice40prog_gpioInit()
{
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOF);
}

// ----------------------------------------------------------------------------
static void ice40prog_spiInit()
{
  // Configure SPI peripheral in master mode
  rcc_periph_clock_enable(RCC_SPI1);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOF);

  // ...
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7);
  gpio_set_af(GPIOA, GPIO_AF0, GPIO5 | GPIO6 | GPIO7);

  // ...
  spi_reset(SPI1);

  // ...
  spi_set_master_mode(SPI1);
  spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_2);
  spi_set_clock_polarity_1(SPI1);
  spi_set_clock_phase_0(SPI1);
  spi_set_full_duplex_mode(SPI1);
  spi_set_data_size(SPI1, SPI_CR2_DS_8BIT);
  spi_enable_software_slave_management(SPI1);
  spi_send_msb_first(SPI1);
  spi_set_nss_high(SPI1);
  spi_fifo_reception_threshold_8bit(SPI1);
  SPI_I2SCFGR(SPI1) &= ~SPI_I2SCFGR_I2SMOD;
  spi_enable(SPI1); 
}

// ----------------------------------------------------------------------------
static void ice40prog_spiRelease()
{
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7);
}

// ----------------------------------------------------------------------------
void ice40prog_init()
{
  // ...
  ice40prog_spiInit();

  // ...
  ice40prog_loadFromSpiFlash(0x1000);
}

// ----------------------------------------------------------------------------
void ice40prog_start()
{
  // ...
  ice40prog_setCSHigh(false);
  ice40prog_fpgaResetHigh(false);
  _delay_ms(5);
  ice40prog_fpgaResetHigh(true);
  _delay_ms(5);
}

// ----------------------------------------------------------------------------
void ice40prog_finish()
{  
  // ...
  int32_t timeout = 250;
  while((ice40prog_cdoneCheck() == 0) && (timeout > 0))
  {
    timeout -= 1;
    ice40prog_spiSend(0x00);
  }

  // ...
  if(timeout == 0)
  {
    ice40prog_setCSHigh(true);
    ice40prog_spiRelease();
    return;
  }

  // Send at least 49 clocks more
  for(uint8_t i;i<7;i++)
  {
    ice40prog_spiSend(0x00);
  }

  // ...
  ice40prog_setCSHigh(true);
}

// ----------------------------------------------------------------------------
void ice40prog_txData(uint8_t* buf, int32_t buflen)
{
  while(buflen--)
  {
    ice40prog_spiSend(*buf++);
  }
}

// ----------------------------------------------------------------------------
void ice40prog_loadFromSpiFlash(uint32_t baseAddress)
{
  uint32_t length;
  uint32_t checksum;
  uint8_t m_buf[256];
  uint8_t m_initialPage = true;
  uint32_t baseOffset = baseAddress;

  ice40prog_start();

  while(true)
  {
    spiFlash_read_data(baseAddress, 256, m_buf);
    baseAddress += 256;

    if(m_initialPage)
    {
      m_initialPage = false;

      // TOOD: Check bitstream checksum with the SPI flash content ...
      checksum = make32b(m_buf, 0);

      // TODO: Check bitstream length is in valid range ...
      length = make32b(m_buf, 4) + 8;

      ice40prog_txData((uint8_t*)(m_buf + 8), 248);
    }
    else
    {
      ice40prog_txData(m_buf, 256);
    } 

    if((length - (baseAddress - baseOffset)) < 256)
    {
      uint32_t remainder = (length - (baseAddress - baseOffset));
      spiFlash_read_data(baseAddress, remainder, m_buf);
      ice40prog_txData(m_buf, remainder);
      break;
    }
  }

  ice40prog_finish();
}
