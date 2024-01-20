DIR_VCD =verilator_simpledev_vcd
CC=gcc
CXX=g++
OBJS_VERILATE='*.sv' # Modify with the rtl object files

# export VERILATOR_ROOT=/home/csx/sw/verilator/v5.016
# generate sv to cpp
verilate_vcd: $(OBJS)
	$(VERILATOR_ROOT)/bin/verilator -Wall -cc --trace Top.sv --Mdir $(DIR_VCD)


build: verilate_vcd wrapper_simdev.cc wrapper_simdev.hh
	g++ -I $(DIR_VCD) -I$(VERILATOR_ROOT)/include wrapper_simdev.cc $(DIR_VCD)/*.cpp \
	$(shell python3 ../extract_verilator.py $(VERILATOR_ROOT)/include/ $(DIR_VCD) VTop.mk) \
	-fpic  -shared -o libVerilatorSimDev.so

install: libVerilatorSimDev.so
	cp libVerilatorSimDev.so ../
	cp wrapper_simdev.hh ../
	cp packet_simdev.hh ../

all: build
	cp libVerilatorSimDev.so ../
	cp wrapper_simdev.hh ../
	cp packet_simdev.hh ../


.PHONY: clean cleanall

clean:
	rm -rf $(DIR_VCD) libVerilatorSimDev.so

cleanall:
	rm -rf $(DIR_VCD) libVerilatorSimDev.so
	rm -rf ../custom_packet_simdev.hh
	rm -rf ../libVerilatorSimDev.so
	rm -rf ../wrapper_simdev.hh