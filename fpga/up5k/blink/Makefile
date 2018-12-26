#------------------------------------------------------------------------------
# 
# 
#------------------------------------------------------------------------------
PROJ     = top
SOURCES  = 
PIN_DEF  = pinmap.pcf
DEVICE   = up5k

#------------------------------------------------------------------------------
# Don't change anything below
#------------------------------------------------------------------------------
OBJDIR = obj
MAKEFLAGS += --no-print-directory

all: $(PROJ).bin

%.blif: %.v
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running yosys ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/yosys_log.txt
	yosys -p 'synth_ice40 -top top -blif $(OBJDIR)/$@' $< $(SOURCES) \
	>> $(OBJDIR)/yosys_log.txt 2>&1 || cat $(OBJDIR)/yosys_log.txt

%.asc: %.blif
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running arachne-pnr ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/arachne-pnr_log.txt
	arachne-pnr -d $(subst up,,$(subst hx,,$(subst lp,,$(DEVICE)))) -o $(OBJDIR)/$@ \
	-p $(PIN_DEF) $(OBJDIR)/$^ >> \
	$(OBJDIR)/arachne-pnr_log.txt 2>&1 || cat $(OBJDIR)/arachne-pnr_log.txt

%.bin: %.asc
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running icepack ..."
	@echo "-------------------------------------------------------------------"
	@rm -rf $(OBJDIR)/icepack_log.txt
	icepack -s $(OBJDIR)/$< $(OBJDIR)/$@ >> $(OBJDIR)/icepack_log.txt 2>&1

prog:
	@echo ""
	@echo "-------------------------------------------------------------------"
	@echo "-- Running iceprog ..."
	@echo "-------------------------------------------------------------------"
	iceprog -S $(OBJDIR)/$(PROJ).bin
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
	make clean && make all -j32 && make prog

iterate_w_timing:
	@echo ""
	@echo "-------------------------------------------------------------------"
	make clean && make all -j32 && make timing && make prog

terminal:
	sudo python -m serial.tools.miniterm /dev/ttyUSB1 1000000

# .SECONDARY:
# .PHONY: all prog clean
-include $(shell mkdir $(OBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)