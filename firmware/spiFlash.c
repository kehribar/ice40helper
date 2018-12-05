// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include "spiFlash.h"
#include "xprintf.h"
#include <string.h>
#include "systick_delay.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// ----------------------------------------------------------------------------
#define spiFlash_cs_high() gpio_set(GPIOF, GPIO1)
#define spiFlash_cs_low() gpio_clear(GPIOF, GPIO1)

// ----------------------------------------------------------------------------
static uint8_t spi_exchange(uint8_t tx)
{
  uint8_t i;
  uint8_t rx = 0;

  for(i=0;i<8;i++)
  {
    rx = rx << 1;

    if(tx & 0x80)
    {
      gpio_set(GPIOA, GPIO2);     
    }
    else
    {
      gpio_clear(GPIOA, GPIO2);     
    }

    gpio_clear(GPIOA, GPIO4); 
    gpio_set(GPIOA, GPIO4);   

    if(gpio_get(GPIOA, GPIO3))
    {
      rx |= 0x01;
    }

    tx = tx << 1;
  }

  return rx;
}

// ----------------------------------------------------------------------------
static void spiFlash_txOnly_spi(uint8_t tx)
{
  uint8_t i;

  for(i=0;i<8;i++)
  {
    if((tx & 0x80) != 0)
    {
      gpio_set(GPIOA, GPIO2);     
    }
    else
    {
      gpio_clear(GPIOA, GPIO2);     
    }

    gpio_clear(GPIOA, GPIO4);   
    gpio_set(GPIOA, GPIO4);   

    tx = tx << 1;
  }
}

// ----------------------------------------------------------------------------
static uint8_t spiFlash_send_spi(uint8_t tx)
{
  uint8_t rx = 0;
  rx = spi_exchange(tx);
  return rx;
}

// ----------------------------------------------------------------------------
void spiFlash_init()
{
  // ...
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOF);

  // TODO: SPI is bitbanged. Change to synchronised UART mode for higher speed
  // transfer.

  // ...
  gpio_mode_setup(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO2);
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT,  GPIO_PUPD_PULLUP, GPIO3);
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO4);

  // ...
  spiFlash_cs_high();

  // ...
  while(spiFlash_check_ID() == 0);
}

// ----------------------------------------------------------------------------
uint8_t spiFlash_check_ID()
{
  uint8_t capacity;
  uint8_t device_type;
  uint8_t manufacturer;

  spiFlash_cs_low();

  // 'Read JEDEC ID' command
  spiFlash_send_spi(0x9F);

  manufacturer = spiFlash_send_spi(0x00);
  device_type = spiFlash_send_spi(0x00);
  capacity = spiFlash_send_spi(0x00);

  spiFlash_cs_high();

  // SPI Flash: AT25SF321
  if(((manufacturer == 0x1F) && (device_type == 0x87)) && (capacity == 0x01))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

// ----------------------------------------------------------------------------
void spiFlash_read_data(uint32_t address, uint32_t length, uint8_t* data)
{
  uint32_t tmp;

  spiFlash_cs_low();

  // Normal speed 'Read Data' command
  spiFlash_txOnly_spi(0x03);

  // A23-A16
  tmp = ((address >> 16) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  // A15-A8
  tmp = ((address >> 8) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  // A7-A0
  tmp = ((address >> 0) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  // Dummy byte ...
  // spiFlash_txOnly_spi(0x00);

  // Fetch data one byte at a time ...
  for(tmp=0; tmp < length; tmp++)
  {
    data[tmp] = spiFlash_send_spi(0x00);
  }

  spiFlash_cs_high();
}

// ----------------------------------------------------------------------------
void spiFlash_write_enable()
{
  spiFlash_cs_low();
  spiFlash_send_spi(0x06);
  spiFlash_cs_high();
}

// ----------------------------------------------------------------------------
void spiFlash_write_disable()
{
  spiFlash_cs_low();
  spiFlash_send_spi(0x04);
  spiFlash_cs_high();
}

// ----------------------------------------------------------------------------
void spiFlash_write_data(uint32_t address, uint32_t length, uint8_t* data)
{
  uint32_t tmp;

  spiFlash_cs_low();

  // 'Page Program' command
  spiFlash_txOnly_spi(0x02);

  // A23-A16
  tmp = ((address >> 16) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  // A15-A8
  tmp = ((address >> 8) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  // A7-A0
  tmp = ((address >> 0) & 0xFF);
  spiFlash_txOnly_spi(tmp);

  for(tmp=0; tmp < length; tmp++)
  {
    // Send data one byte at a time ...
    spiFlash_txOnly_spi(data[tmp]);
  }

  spiFlash_cs_high();
}

// ----------------------------------------------------------------------------
void spiFlash_sectorErase(uint32_t address)
{
  uint32_t tmp;

  spiFlash_cs_low();

  // Erase 4kB worth of data with 'Sector Erase' command
  spiFlash_send_spi(0x20);

  // A23-A16
  tmp = ((address >> 16) & 0xFF);
  spiFlash_send_spi(tmp);

  // A15-A8
  tmp = ((address >> 8) & 0xFF);
  spiFlash_send_spi(tmp);

  // A7-A0
  tmp = ((address >> 0) & 0xFF);
  spiFlash_send_spi(tmp);

  spiFlash_cs_high();
}

// ----------------------------------------------------------------------------
uint8_t spiFlash_readStatus()
{
  uint8_t tmp;

  spiFlash_cs_low();
  spiFlash_send_spi(0x05);
  tmp = spiFlash_send_spi(0x00);
  spiFlash_cs_high();

  return tmp;
}

// ----------------------------------------------------------------------------
uint8_t spiFlash_isBusy()
{
  if(spiFlash_readStatus() & 0x01)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

// ----------------------------------------------------------------------------
static uint32_t m_baseAddress;
static uint32_t m_flashBufferInd;
static uint8_t m_flashBuffer[256];

// ----------------------------------------------------------------------------
void spiFlash_write_initalise(uint32_t address)
{
  m_flashBufferInd = 0;
  m_baseAddress = address;
}

// ----------------------------------------------------------------------------
void spiFlash_write_push(uint8_t* data, uint32_t length)
{
  for(int32_t i=0;i<length;i++)
  {
    m_flashBuffer[m_flashBufferInd] = data[i];
    m_flashBufferInd += 1;

    if(m_flashBufferInd == 256)
    {
      spiFlash_write_enable();
      spiFlash_write_data(m_baseAddress, 256, m_flashBuffer);
      while(spiFlash_isBusy());
      spiFlash_write_disable();
      m_baseAddress += 256;
      m_flashBufferInd = 0;
    }
  }
}

// ----------------------------------------------------------------------------
void spiFlash_write_finalise()
{
  spiFlash_write_enable();
  spiFlash_write_data(m_baseAddress, 256, m_flashBuffer);
  while(spiFlash_isBusy());
  spiFlash_write_disable();
}
