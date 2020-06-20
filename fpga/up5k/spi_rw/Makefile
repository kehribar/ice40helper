#------------------------------------------------------------------------------
# 
# 
#------------------------------------------------------------------------------
PROJ     = top
SOURCES  = ./common/cmdspi/cmdspi.v ./common/debounce.v
DEVICE   = up5k
PIN_DEF  = pinmap.pcf
PORT     = /dev/ttyACM1
F_CLOCK  = 32

#------------------------------------------------------------------------------
# Don't change anything below
#------------------------------------------------------------------------------
OBJDIR = obj
MAKEFLAGS += --no-print-directory

all: $(PROJ).bin

%.json: %.v
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running yosys ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/yosys_log.txt
	yosys -p 'synth_ice40 -noflatten -abc2 -json $(OBJDIR)/$@ -top top' $< $(SOURCES) \
	>> $(OBJDIR)/yosys_log.txt 2>&1 || cat $(OBJDIR)/yosys_log.txt

%.asc: %.json
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running nextpnr ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/arachne-pnr_log.txt
	nextpnr-ice40 --$(DEVICE) --asc $(OBJDIR)/$@ --pcf $(PIN_DEF) \
	--freq $(F_CLOCK) --json $(OBJDIR)/$^ >> $(OBJDIR)/nextpnr-pnr_log.txt 2>&1 \
	--opt-timing || cat $(OBJDIR)/nextpnr_log.txt

%.bin: %.asc
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running icepack ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/icepack_log.txt
	icepack -s $(OBJDIR)/$< $(OBJDIR)/$@ >> $(OBJDIR)/icepack_log.txt 2>&1

gui:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running nextpnr (GUI)..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/arachne-pnr_log.txt
	nextpnr-ice40 --$(DEVICE) --asc $(OBJDIR)/$(PROJ).asc --pcf $(PIN_DEF) \
	--json $(OBJDIR)/$(PROJ).json --gui

prog:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Programming (SRAM) ..."
	@echo "-------------------------------------------------------------------"
	ice40prog $(PORT) ./obj/top.bin
	@echo ""

prog_flash:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Programming (Flash) ..."
	@echo "-------------------------------------------------------------------"
	ice40prog_flash $(PORT) ./obj/top.bin
	@echo ""

clean:
	@rm -rf obj

timing:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running icetime ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/icetime_log.txt
	icetime -d $(DEVICE) -mtr $(OBJDIR)/$(PROJ).rpt $(OBJDIR)/$(PROJ).asc \
	>> $(OBJDIR)/icetime_log.txt 2>&1	
	@echo ""
	@cat $(OBJDIR)/icetime_log.txt 

iterate:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "--"
	@echo "-------------------------------------------------------------------"
	make clean && make all -j32 && make prog

iterate_w_timing:
	@echo ""
	@echo "-------------------------------------------------------------------"
	make clean && make all -j32 && make timing && make prog

# .SECONDARY:
# .PHONY: all prog clean
-include $(shell mkdir $(OBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)
