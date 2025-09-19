PROJECT=rom
ACLI=~/acli/bin/arduino-cli
PLATFORM=rp2040:rp2040:rpipico
PORT=/dev/ttyACM0
UF2=build/${PROJECT}.uf2
ELF=build/${PROJECT}.elf
MOUNT=/media/rp2040

OBJS=rom.c

default_target: ${ELF}
.PHONY : default_target

make:
	cmake -S . -B build -DPICO_SDK_PATH=pico-sdk/

${ELF}: ${OBJS}
	make -C build

mc: 	# mount and copy
	sudo ./mntrp2040.sh
	sudo cp ${UF2} ${MOUNT}
#	sudo umount ${MOUNT}
	

upload: 
	${ACLI} upload  -b ${PLATFORM} -p ${PORT} -i  ${UF2}

s:
	screen ${PORT} 115200
