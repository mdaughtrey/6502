PROJECT=ioutils
ASMS=main.asm chaser.asm varstack.asm i2c.asm via6522.asm lcd.asm
OBJS=$(ASMS:.asm=.o)

# .phony: compile

$(PROJECT): $(OBJS)
	ld65 --dbgfile rom1.dbg -C ${PROJECT}.cfg --mapfile $(PROJECT).map -o $(PROJECT).bin $(OBJS)
	xxd -i rom1.bin > ${PROJECT}.h
	cp ${PROJECT}.h ../../rom/src

%.o: %.asm
	ca65 --verbose -g --cpu 65c02 --listing $(<).lst -o $@ $<

dump:
	xxd -i rom1.bin > ${PROJECT}.h

clean:
	rm -f *.o *.bin *.lst


