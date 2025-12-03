.include "mcp23017.inc"

;.import I2C_ADDR, I2C_BUFFER, I2C_DEVICE 
.import i2c_byte_to_addr 
.import var_push

.export chaser_init, chaser_loop

.segment "DATA"
mycount: .byte 1

.segment "CODE"
.proc chaser_init
    lda #$00         ; Set IODIRA to outputs
    jsr var_push
    lda #IODIRA    ; Reg address
    jsr var_push
    lda #$20        ; I2C Device
    jsr var_push
    jsr i2c_byte_to_addr
    rts
.endproc


.proc chaser_loop
    lda mycount
    jsr var_push
    lda #GPIOA
    jsr var_push
    lda #$20        ; I2C Device
    jsr var_push
    jsr i2c_byte_to_addr
    inc mycount
    rts
.endproc

