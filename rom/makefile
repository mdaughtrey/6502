PROJECT=rom
ACLI=~/acli/bin/arduino-cli
PLATFORM=rp2040:rp2040:rpipico
PORT=/dev/ttyACM0
PICOPROBE_PORT=/dev/ttyACM1
UF2=build/${PROJECT}.uf2
ELF=build/${PROJECT}.elf
MOUNT=/media/rp2040

OBJS=src/rom.cpp src/menu.cpp src/io_menu.cpp

.PHONY : compile


compile: 
	PICO_SDK_PATH=~/dev/pico-sdk/ make -C build

build:
	cmake -S . -B build -DPICO_SDK_PATH=~/dev/pico-sdk/ -DCMAKE_BUILD_TYPE=Debug -DPICO_PLATFORM=rp2350

clean:
	rm -rf build

mc: 	# mount and copy
	sudo ./mntrp2040.sh
	sudo cp ${UF2} ${MOUNT}
	sudo umount ${MOUNT}
	

upload: 
	${ACLI} upload  -b ${PLATFORM} -p ${PORT} -i  ${UF2}

s:
	screen ${PORT} 115200

# openocd -s /usr/share/openocd/ -f scripts/interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 100"
# gdb ~/bin/openocd/src/openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg  -s ~/bin/openocd/tcl
# ~/bin/openocd/src/openocd -s ~/bin/openocd/tcl -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program build/app.elf verify reset exit"
# ~/bin/openocd/src/openocd -s ~/bin/openocd/tcl -f interface/cmdis-dap.cfg -f target/rp2040.cfg -c "program build/app.elf verify reset exit"
