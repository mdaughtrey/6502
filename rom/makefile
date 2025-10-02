PROJECT=rom
ACLI=~/acli/bin/arduino-cli
PLATFORM=rp2040:rp2040:rpipico
PORT=/dev/ttyACM0
UF2=build/${PROJECT}.uf2
ELF=build/${PROJECT}.elf
MOUNT=/media/rp2040

OBJS=src/rom.cpp src/menu.cpp src/io_menu.cpp

.PHONY : compile


compile: 
	PICO_SDK_PATH=~/dev/pico-sdk/ make -C build

build:
	cmake -S . -B build -DPICO_SDK_PATH=~/dev/pico-sdk/ 

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
