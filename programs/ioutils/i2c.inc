;.include    "io.inc"

.macro scl_high
    ora #I2C_SCL
    sta SELECTPORT
.endmacro

.macro scl_low
    and #NOT_I2C_SCL
    sta SELECTPORT
.endmacro

.macro sda_high
    ora #I2C_SDA
    sta SELECTPORT
.endmacro

.macro sda_low
    and #NOT_I2C_SDA
    sta SELECTPORT
.endmacro

I2C_SCL = %10000000    ; 6522 SELECTPORT D7
NOT_I2C_SCL = %01111111
I2C_SDA = %01000000    ; 6522 SELECTPORT D6
NOT_I2C_SDA = %10111111

.segment "ZEROPAGE"
I2C_DEVICE:     .byte 0
I2C_ADDR:       .byte 0
I2C_BUFFER_INDEX: .byte 0
I2C_BUFFER_LENGTH: .byte 0
I2C_BUFFER: .res 8

.segment "CODE"
.proc   i2c_init
    lda SELECTPORT
    sda_high
    scl_low
    sta SELECTPORT
    rts
.endproc


; I2C_DATA0 = byte to write
.proc i2c_write_byte
    rts
.endproc

.proc i2c_read_byte
    rts
.endproc

.proc i2c_bytes_to_addr
    rts
.endproc

.proc i2c_byte_to_addr
    rts
.endproc


; Read data from addr
; I2C_DEVICE, 
.proc i2c_byte_from_addr
    ; Set start condition
    lda SELECTPORT      
    scl_low
    sda_high
    scl_high
    sda_low
    scl_low

    ; Write out the address
    lda I2C_DEVICE
    rol
    tay                 ; y = data
    lda #7              ; Address is 7 bits
    tax                 ; x = #bits
device_bitloop:
    tya                 ; load address
    rol a               ; rotate next bit into carry flag
    tay                 ; store data
    bcs device_high_bit   ; carry = high bit
    lda SELECTPORT
    sda_low
    jmp device_clockit
device_high_bit:
    lda SELECTPORT
    sda_high
device_clockit:
    scl_high
    scl_low
    dex
    bne device_bitloop

    ; Set Write Transaction
    ; sda_high
    sda_low
    scl_high
    scl_low

    ; Read ACK/NAK
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
    lda SELECTPORT
    scl_high
    ; read dataport for ack/nak
    scl_low
    lda SELECTIO        ; set SDA to output
    ora #I2C_SDA
    sta SELECTIO

    ; Write out the address
    lda I2C_ADDR
    tay
    lda #8              ; Address is 8 bits
    tax
addr_bitloop:
    tya                 ; load address
    rol a               ; rotate next bit into carry flag
    tay                 ; store data
    bcs addr_high_bit   ; carry = high bit
    lda SELECTPORT
    sda_low
    jmp addr_clockit
addr_high_bit:
    lda SELECTPORT
    sda_high
addr_clockit:
    scl_high
    scl_low
    dex
    bne addr_bitloop

    ; Read ACK/NAK
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
    lda SELECTPORT
    scl_high            ; clock in the ack/nak
    scl_low
    lda SELECTIO        ; set SDA to output
    ora #I2C_SDA
    sta SELECTIO

    ; Read in the register value
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
    lda #8              ; Address is 8 bits
    tax
reg_bitloop:
    lda SELECTPORT
    scl_high
    rol
    rol                 ; C = received bit
    bcs reg_high_bit
    asl I2C_BUFFER
    jmp reg_clockit
reg_high_bit:
    lda I2C_BUFFER
    asl
    ora #1
    sta I2C_BUFFER
reg_clockit:
    lda SELECTPORT
    scl_low
    dex
    bne reg_bitloop

    ; Read ACK/NAK
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
    lda SELECTPORT
    scl_high            ; clock in the ack/nak
    scl_low
    lda SELECTIO        ; set SDA to output
    ora #I2C_SDA
    sta SELECTIO

    ; Stop Condition
stop:
    lda SELECTPORT      ; stop condition, SCL is Low
    eor #I2C_SDA         ; SDA Low
    sta SELECTPORT      ; ...
    ora #I2C_SCL         ; SCL High
    sta SELECTPORT      ; ...
    ora #I2C_SDA        ; SDA High
    sta SELECTPORT      ; ...
    eor #I2C_SCL        ; SCK Low
    sta SELECTPORT      ; ...

    rts
.endproc

.proc i2c_bytes_from_addr
    rts
.endproc

; SDA is low during clock cycle
.proc i2c_writebit
    rts
.endproc

; SDA is high during clock cycle
.proc i2c_readbit
    rts
.endproc

; Write out an N bit frame
; stack0: numbits
; stack1: data
.proc i2c_frame
    rts
.endproc

; Write out a frame start condition
.proc i2c_start
    rts
.endproc

; Write out a frame stop condition
.proc i2c_stop
    rts
.endproc
