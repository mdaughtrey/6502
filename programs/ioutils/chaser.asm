.include "mcp23017.inc"

.import I2C_ADDR, I2C_BUFFER, I2C_DEVICE 
.import i2c_byte_to_addr 

.proc chaser_init
    lda $20
    sta I2C_DEVICE
    lda #IODIRA
    sta I2C_ADDR
    lda #$ff
    sta I2C_BUFFER
    jsr i2c_byte_to_addr
    rts
.endproc


.proc chaser_loop
    lda $20
    sta I2C_DEVICE
    
    rts
.endproc
