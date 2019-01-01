#!/usr/bin/python3
# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------
import sys
import time
import serial
import struct
import numpy as np

# -----------------------------------------------------------------------------
sp = None

# -----------------------------------------------------------------------------
def debug_buf(buf):
  print(' '.join('%02X'%i for i in buf))

#------------------------------------------------------------------------------
def make32b(buff, offset):
  rv  = buff[offset + 0]
  rv += buff[offset + 1] << 8
  rv += buff[offset + 2] << 16
  rv += buff[offset + 3] << 24
  return rv

#------------------------------------------------------------------------------
def put32b(buff, offset, val):
  buff[offset + 0] = (val & 0x000000FF) >> 0
  buff[offset + 1] = (val & 0x0000FF00) >> 8
  buff[offset + 2] = (val & 0x00FF0000) >> 16
  buff[offset + 3] = (val & 0xFF000000) >> 24

# -----------------------------------------------------------------------------
def writeRegister32b(address, data):
  buff = bytearray(128)
  buff[0] = 9
  buff[1] = address
  put32b(buff, 2, data)
  sp.write(buff)
  rbuf = sp.read(64)
  # debug_buf(rbuf[0:4])

# -----------------------------------------------------------------------------
def readRegister32b(address):
  buff = bytearray(128)
  buff[0] = 10
  buff[1] = address
  sp.write(buff)
  rbuf = sp.read(64)
  if(len(rbuf) == 64):
    retval = make32b(rbuf, 0)
    return retval
  else:
    print(len(rbuf))
    return -1

# -----------------------------------------------------------------------------
if __name__ == '__main__':

  # ...
  sp = serial.Serial(port = sys.argv[1], baudrate = 999999999, timeout = 0.1)
  if(sp.isOpen() == True):
    sp.close()
  sp.open()


  counter = 0
  while True:
    counter += 1
    print("")

    writeRegister32b(0x00, 0x12345678)
    retval = readRegister32b(0x00)
    print(hex(retval).upper())

    writeRegister32b(0x00, 0xFEEDF00D)
    retval = readRegister32b(0x00)
    print(hex(retval).upper())

    writeRegister32b(0x00, counter)
    retval = readRegister32b(0x00)    
    print(hex(retval).upper())

    retval = readRegister32b(0x10)
    print(hex(retval).upper())
