OBJ=chaser

.phony: compile

compile:
	ca65 --listing ${OBJ}.lst ${OBJ}.asm
	ld65 -C ${OBJ}.cfg -o $(OBJ).bin $(OBJ).o

dump:
	xxd -c 16 -g 1 rom1.bin | head -4

clean:
	rm -f *.o *.bin


