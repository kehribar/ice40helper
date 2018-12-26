// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include "spiFlash.h"
#include "xprintf.h"
#include <string.h>
#include "systick_delay.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

// ----------------------------------------------------------------------------
#define spiFlash_cs_high() gpio_set(GPIOA, GPIO1)
#define spiFlash_cs_low() gpio_clear(GPIOA, GPIO1)
#define spiFlash_send_spi(tx) spi_exchange(tx)

// ----------------------------------------------------------------------------
static uint8_t spi_exchange(uint8_t tx)
{
  // ...
  while((USART2_ISR & USART_ISR_TXE) == 0);
  USART2_TDR = tx;
  
  // ...
  while((USART2_ISR & USART_ISR_RXNE) == 0);
  return USART2_RDR;
}

// ----------------------------------------------------------------------------
static void spiFlash_txOnly_spi(uint8_t tx)
{
  volatile uint8_t devNull;
  
  // ...
  while((USART2_ISR & USART_ISR_TXE) == 0);
  USART2_TDR = tx;

  // ...
  while((USART2_ISR & USART_ISR_RXNE) == 0);
  devNull = USART2_RDR;
}

// ----------------------------------------------------------------------------
void spiFlash_init()
{
  // ...
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART2);

  // ...
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO1);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3 | GPIO4);
  gpio_set_af(GPIOA, GPIO_AF1, GPIO2 | GPIO3 | GPIO4);

  // ...
  USART2_CR1 = USART_CR1_RE | USART_CR1_OVER8;
  USART2_CR2 = USART_CR2_SWAP | USART_CR2_CLKEN | USART_CR2_MSBFIRST | USART_CR2_LBCL;

  // 6MBps with 48MHz CPU clock
  USART2_BRR = 0x10;

  // ...
  USART2_CR1 |= USART_CR1_UE;

  // ...
  USART2_CR1 |= USART_CR1_TE;

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

  // xprintf("\r\n");
  // xprintf("manufacturer: %2X\r\n", manufacturer);
  // xprintf("device_type:  %2X\r\n", device_type);
  // xprintf("capacity:     %2X\r\n", capacity);

  // SPI Flash: AT25SF041
  if(((manufacturer == 0x1F) && (device_type == 0x84)) && (capacity == 0x01))
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
  spiFlash_send_spi(0x03);

  // A23-A16
  tmp = ((address >> 16) & 0xFF);
  spiFlash_send_spi(tmp);

  // A15-A8
  tmp = ((address >> 8) & 0xFF);
  spiFlash_send_spi(tmp);

  // A7-A0
  tmp = ((address >> 0) & 0xFF);
  spiFlash_send_spi(tmp);

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
