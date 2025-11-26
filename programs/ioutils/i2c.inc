;.include    "io.inc"

I2C_SCK = %10000000    ; 6522 SELECTPORT D7
I2C_SDA = %01000000    ; 6522 SELECTPORT D6

.segment "CODE"
.proc   i2c_init
:
    lda SELECTPORT
    ora #I2C_SCK
    sta SELECTPORT
    ora #I2C_SDA
    sta SELECTPORT

    eor #I2C_SDA
    sta SELECTPORT
    eor #I2C_SCK
    sta SELECTPORT
    jmp :-
    rts
.endproc

.proc write_byte
    rts
.endproc

.proc read_byte
    rts
.endproc

.proc byte_to_addr
    rts
.endproc

.proc byte_from_addr
    rts
.endproc

.proc bytes_to_addr
    rts
.endproc

.proc bytes_from_addr
    rts
.endproc
