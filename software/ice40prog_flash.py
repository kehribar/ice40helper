# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------
import sys
import time
import json
import serial
import numpy as np
from intelhex import IntelHex
from datetime import datetime

# -----------------------------------------------------------------------------
def put32b(data):
  rv = bytearray(4)
  rv[0] = (data & 0x000000FF) >> 0
  rv[1] = (data & 0x0000FF00) >> 8
  rv[2] = (data & 0x00FF0000) >> 16
  rv[3] = (data & 0xFF000000) >> 24
  return rv

# -----------------------------------------------------------------------------
if __name__ == '__main__':

  # ...
  sp = serial.Serial(port = sys.argv[1], baudrate = 999999999, timeout = 0.1)
  if(sp.isOpen() == True):
    sp.close()
  sp.open()

  # ..
  ih = IntelHex()
  ih.loadbin(sys.argv[2])
  binbuff = ih.todict()

  tmp = bytearray(len(binbuff))
  for i in range(len(binbuff)):
    tmp[i] = binbuff[i]
  binbuff = tmp

  buflen = len(binbuff)
  checksum = sum(binbuff)

  binbuff = put32b(buflen) + binbuff
  binbuff = put32b(checksum) + binbuff

  # ...
  TEST_LEN = 128
  buf = [0] * TEST_LEN
  tmpbuf = [0] * TEST_LEN
  buf = bytearray(buf)
  tmpbuf = bytearray(tmpbuf)

  # Start the process
  x = 0
  print("Flash erase ...")
  while True:
    if((x * 4096) < len(binbuff)):
      tmpbuf[0] = 4
      tmpbuf[1] = x + 1
      sp.write(tmpbuf)
      rbuf = sp.read(64)
      x = x + 1
    else:
      break
  print("Flash erase finished!")

  # ...
  last_delta = 0
  bytes_total = 0
  t1 = datetime.now()

  print("Initialise spi flash write")
  tmpbuf[0] = 5
  tmpbuf[1:5] = put32b(0x1000);
  sp.write(tmpbuf)
  rbuf = sp.read(64)

  # ...
  bin_offset = 0
  final_packet = False

  # Transfer binary file ...
  print("Transfer ...")
  BYTES_PER_PACKET = 127
  while True:

    # Assemble a packet
    buf[0] = 6
    base_ind = bin_offset
    for x in range(0,BYTES_PER_PACKET):
      ind = base_ind + x
      if(ind == len(binbuff)):
        final_packet = True
        break
      else:
        buf[x + 1] = binbuff[ind]
    bin_offset += BYTES_PER_PACKET

    # Send and wait for a response
    sp.write(buf)
    rbuf = sp.read(64)
    bytes_total += len(buf)

    if(final_packet):
      break
  
  # Finish the process 
  print("Finalise spi flash write")
  tmpbuf[0] = 7
  sp.write(tmpbuf)
  rbuf = sp.read(64)

  print("Load bitstream ...")
  tmpbuf[0] = 8
  tmpbuf[1:5] = put32b(0x1000);
  sp.write(tmpbuf)
  rbuf = sp.read(64)

  # ...
  t2 = datetime.now()
  delta = t2 - t1
  delta = delta.total_seconds()
  speed = bytes_total / delta
  speed /= 1024

  # ...
  print("Transfer finished!")
  print("File size:", len(binbuff), "bytes")
  print("Tranfer speed:", int(speed), "kB/sec")
  print("Total duration:", int(delta * 1000), "ms");
