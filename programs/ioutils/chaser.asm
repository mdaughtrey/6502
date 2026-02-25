re.include "via6522_regs.inc"


;.import I2C_ADDR, I2C_BUFFER, I2C_DEVICE 
;.import i2c_byte_to_addr 
;.import var_push

.export chaser_init, chaser_loop

.segment "DATA"
mycount: .byte 1

.segment "CODE"
.proc chaser_init
    lda #$ff
    sta DDRA
    sta DDRB
    rts
.endproc


.proc chaser_loop
    lda mycount
    sta ORAIRA
    sta ORBIRB
    inc mycount
    rts
.endproc

