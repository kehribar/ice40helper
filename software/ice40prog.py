# -----------------------------------------------------------------------------
# 
# -----------------------------------------------------------------------------
import sys
import time
import serial
import numpy as np
from intelhex import IntelHex
from datetime import datetime

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

  # ...
  TEST_LEN = 128
  buf = [0] * TEST_LEN
  tmpbuf = [0] * TEST_LEN
  buf = bytearray(buf)
  tmpbuf = bytearray(tmpbuf)

  # Start the process
  tmpbuf[0] = 2
  sp.write(tmpbuf)
  rbuf = sp.read(64)

  # ...
  bin_offset = 0
  final_packet = False

  # ...
  last_delta = 0
  bytes_total = 0
  t1 = datetime.now()

  # Transfer binary file ...
  while True:

    # Assemble a packet
    buf[0] = 1
    base_ind = bin_offset
    for x in range(0,127):
      ind = base_ind + x
      if(ind == len(binbuff)):
        final_packet = True
        break
      else:
        buf[x + 1] = binbuff[ind]
    bin_offset += 127

    # Send and wait for a response
    sp.write(buf)
    rbuf = sp.read(64)
    bytes_total += len(buf)

    if(final_packet):
      break
  
  # Finish the process 
  tmpbuf[0] = 3
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
  print("  File size:     ", len(binbuff), "bytes")
  print("  Tranfer speed: ", int(speed), "kB/sec")
  print("  Total duration:", int(delta * 1000), "ms");
