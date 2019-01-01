// ----------------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include "usb_descriptions.h"
#include "usb.h"
#include "ringBuffer.h"
#include "systick_delay.h"

// ----------------------------------------------------------------------------
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

// ---------------------------------------------------------------------------
static usbd_device* usb_dev;
static uint8_t usbd_control_buffer[128];

// ---------------------------------------------------------------------------
static uint8_t usb_txBuf[512];
static uint8_t usb_rxBuf[512];
static RingBuffer_t usb_txBuf_rb;
static RingBuffer_t usb_rxBuf_rb;
static uint8_t m_usbIsConnected = false;

// ---------------------------------------------------------------------------
static enum usbd_request_return_codes cdcacm_control_request(
  usbd_device *usbd_dev,
  struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
  void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req)
)
{
  // --------------------------------------------------------------------------
  switch(req->bRequest)
  {
    // ------------------------------------------------------------------------
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
    {
      // DTR is bit 0, RTS is bit 1
      uint16_t rtsdtr = req->wValue;
      m_usbIsConnected = rtsdtr & 0x01;
      return USBD_REQ_HANDLED;
    }
    // ------------------------------------------------------------------------
    case USB_CDC_REQ_SET_LINE_CODING:
    {
      if((*len) < sizeof(struct usb_cdc_line_coding))
      {
        return USBD_REQ_NOTSUPP;
      }
      return USBD_REQ_HANDLED;
    }
  }

  return USBD_REQ_NOTSUPP;
}

// ---------------------------------------------------------------------------
static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
  uint8_t buf[64];

  int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for(int32_t i=0;i<len;i++)
    {
      if(!RingBuffer_IsFull(&usb_rxBuf_rb))
      {
        RingBuffer_Insert(&usb_rxBuf_rb, buf[i]);
      }
    }
  }
}

// ---------------------------------------------------------------------------
static void usb_txData_actual()
{
  uint8_t buf[64];
  uint8_t buflen = 0;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for(int32_t i=0;i<64;i++)
    {
      if(RingBuffer_IsEmpty(&usb_txBuf_rb))
      {
        break;
      }

      buflen += 1;
      buf[i] = RingBuffer_Remove(&usb_txBuf_rb);
    }
  }

  // ...
  if(buflen > 0)
  {
    while(usbd_ep_write_packet(usb_dev, 0x82, buf, buflen) == 0);
  }
}

// ---------------------------------------------------------------------------
static void cdcacm_data_tx_cb(usbd_device *usbd_dev, uint8_t ep)
{
  if(RingBuffer_IsEmpty(&usb_txBuf_rb))
  {
    uint8_t buf;
    usbd_ep_write_packet(usb_dev, 0x82, &buf, 0);
  }
  else
  {
    usb_txData_actual();    
  }
}

// ---------------------------------------------------------------------------
static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
  usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
  usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_tx_cb);
  usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

  // ...
  usbd_register_control_callback(
    usbd_dev,
    USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
    USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
    cdcacm_control_request
  );
}

// ---------------------------------------------------------------------------
void usb_init()
{
  // ...
  rcc_periph_clock_enable(RCC_USB);
  rcc_periph_clock_enable(RCC_GPIOA);

  // ...
  RingBuffer_InitBuffer(&usb_txBuf_rb, usb_txBuf, sizeof(usb_txBuf));
  RingBuffer_InitBuffer(&usb_rxBuf_rb, usb_rxBuf, sizeof(usb_rxBuf));

  // ...
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
  gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

  // ...
  usb_dev = usbd_init(
    &st_usbfs_v2_usb_driver, &dev, &config, usb_strings,
    3, usbd_control_buffer, sizeof(usbd_control_buffer)
  );

  // ...
  usbd_register_set_config_callback(usb_dev, cdcacm_set_config);
}

// ---------------------------------------------------------------------------
void usb_poll()
{
  usbd_poll(usb_dev);
}

// ---------------------------------------------------------------------------
uint8_t usb_isConnected()
{
  return m_usbIsConnected;
}

// ---------------------------------------------------------------------------
int32_t usb_rxDataAmount()
{
  int32_t rv;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    rv = RingBuffer_GetCount(&usb_rxBuf_rb);
  }
  return rv;
}

// ---------------------------------------------------------------------------
void usb_rxData(uint8_t* buf, int32_t buflen)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for(int32_t i=0;i<buflen;i++)
    {
      if(RingBuffer_IsEmpty(&usb_rxBuf_rb))
      {
        // ...
      }
      else
      {
        buf[i] = RingBuffer_Remove(&usb_rxBuf_rb);
      }
    }
  }
}

// ---------------------------------------------------------------------------
void usb_txData(uint8_t* buf, int32_t buflen)
{
  uint8_t m_triggerFlag = false;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if(RingBuffer_IsEmpty(&usb_txBuf_rb))
    {
      m_triggerFlag = true;
    }

    for(int32_t i=0;i<buflen;i++)
    {
      if(!RingBuffer_IsFull(&usb_txBuf_rb))
      {
        RingBuffer_Insert(&usb_txBuf_rb, buf[i]);
      }
    }
  }

  if(m_triggerFlag)
  {
    usb_txData_actual();
  }
}

// ---------------------------------------------------------------------------
void usb_txData_direct(uint8_t* buf, int32_t buflen)
{
  while(usbd_ep_write_packet(usb_dev, 0x82, buf, buflen) == 0);
}

// ---------------------------------------------------------------------------
void usb_sendChar(uint8_t ch)
{
  if(usb_isConnected())
  {
    usb_txData(&ch, 1);
  }
}
