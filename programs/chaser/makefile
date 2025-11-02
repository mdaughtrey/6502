OBJ=chaser

.phony: compile

compile:
	ca65 --listing ${OBJ}.lst ${OBJ}.asm
	ld65 -C ${OBJ}.cfg -o $(OBJ).bin $(OBJ).o

dump:
	./dump.sh ${OBJ}

clean:
	rm -f *.o *.bin


