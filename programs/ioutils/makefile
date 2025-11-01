PROJECT=ioutils
OBJ=ioutils

.phony: compile

compile:
	ca65 --verbose --listing ${PROJECT}.lst ${OBJ}.asm
	ld65 -C ${PROJECT}.cfg -o $(PROJECT).bin $(OBJ).o
	xxd -i rom1.bin > ${PROJECT}.h
	cp ${PROJECT}.h ../../rom/src

dump:
	xxd -i rom1.bin > ${PROJECT}.h

clean:
	rm -f *.o *.bin


