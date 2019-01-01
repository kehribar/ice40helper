# -----------------------------------------------------------------------------
# 
# 
# -----------------------------------------------------------------------------

PROJECT = main

# .. 
SHARED_DIR = 

# ...
CFILES  = main.c usb.c systick_delay.c xprintf.c
CFILES += ice40prog.c comHandler.c spiFlash.c
CFILES += bytesutil.c ice40data.c

# ...
CFLAGS += -Wno-unused-parameter -Wno-strict-prototypes
CFLAGS += -Wno-missing-prototypes

# ...
OOCD_FILE = 
DEVICE = stm32f042F6

# ...
BUILD_DIR = obj
VPATH += $(SHARED_DIR)
OPENCM3_DIR = /home/kehribar/libopencm3
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))

# ...
include $(OPENCM3_DIR)/mk/genlink-config.mk
include ./rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

prog:
	# st-flash --reset write ./obj/main.bin 0x8000000
	sudo dfu-util --reset --device 0483:DF11 -D ./obj/main.bin -s 0x08000000:32768 -a 0

iterate:
	make all -j32 && rm *.ld
	mv $(PROJECT).bin ./obj
	mv $(PROJECT).elf ./obj
	arm-none-eabi-size ./obj/$(PROJECT).elf
	make prog

monitor:
	screen /dev/ttyACM0 
	