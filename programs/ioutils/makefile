PROJECT=ioutils
ASMS=main.asm chaser.asm varstack.asm i2c.asm via6522.asm
OBJS=$(ASMS:.asm=.o)

# .phony: compile

$(PROJECT): $(OBJS)
	ld65 -C ${PROJECT}.cfg --mapfile $(PROJECT).map -o $(PROJECT).bin $(OBJS)
	xxd -i rom1.bin > ${PROJECT}.h
	cp ${PROJECT}.h ../../rom/src

%.o: %.asm
	ca65 --verbose --listing $(<).lst -o $@ $<

dump:
	xxd -i rom1.bin > ${PROJECT}.h

clean:
	rm -f *.o *.bin *.lst


