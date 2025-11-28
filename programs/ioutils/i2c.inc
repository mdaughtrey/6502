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

.macro sda_in
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
.endmacro

.macro sda_out
    lda SELECTIO        ; set SDA to input
    ora #I2C_SDA
    sta SELECTIO
.endmacro

.macro i2c_start
    lda SELECTPORT      
    scl_low
    sda_high
    scl_high
    sda_low
    scl_low
.endmacro

.macro i2c_stop
    lda SELECTPORT      ; stop condition, SCL is Low
    eor #I2C_SDA         ; SDA Low
    sta SELECTPORT      ; ...
    ora #I2C_SCL         ; SCL High
    sta SELECTPORT      ; ...
    ora #I2C_SDA        ; SDA High
    sta SELECTPORT      ; ...
    eor #I2C_SCL        ; SCK Low
    sta SELECTPORT      ; ...
.endmacro

I2C_SCL = %10000000    ; 6522 SELECTPORT D7
NOT_I2C_SCL = %01111111
I2C_SDA = %01000000    ; 6522 SELECTPORT D6
NOT_I2C_SDA = %10111111

.segment "ZEROPAGE"
I2C_DEVICE:     .byte 0     ; I2C Device Address (7 bits, unshifted)
I2C_ADDR:       .byte 0     ; I2C Register Address
I2C_FLAGS:      .byte 0     ; Control flags
I2C_FLAG_READ   = %00000001
I2C_FLAG_WRITE  = %00000010
I2C_NUM_BITS:    .byte 0     ; number of bits to transact
I2C_BUFFER_INDEX: .byte 0   ; read/write buffer index
I2C_BUFFER_LENGTH: .byte 0  ; number of bytes processed
I2C_BUFFER: .res 8          ; read/write/buffer

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

; ---------------------------------------------
; Transact a frame
; I2C_BUFFER = data byte to write if
; I2C_NUM_BITS = data length in bits
; I2C_FLAGS = read/write
; ---------------------------------------------
.proc i2c_write
    lda I2C_BUFFER
    tay                 ; y = data
    lda I2C_NUM_BITS     ; Address is 7 bits
    tax                 ; x = #bits
bitloop:
    tya                 ; load data
    rol a               ; rotate next bit into carry flag
    tay                 ; store data
    bcs high_bit   ; carry = high bit
    lda SELECTPORT
    sda_low
    jmp clock_bit
high_bit:
    lda SELECTPORT
    sda_high
clock_bit:          ; clock out the current bit
    scl_high
    scl_low
    dex
    bne bitloop

    ; Set Read/Write Transaction
    lda I2C_FLAGS
    and #I2C_FLAG_READ
    cmp #I2C_FLAG_READ
    beq read

    lda I2C_FLAGS
    and #I2C_FLAG_WRITE
    cmp #I2C_FLAG_WRITE
    beq write
    jmp ack_nak
read:
    sda_high
    jmp rw_clock
write:
    sda_low
rw_clock:
    scl_high
    scl_low

ack_nak:
    ; Read ACK/NAK
    sda_in
    lda SELECTPORT
    scl_high
    ; read dataport for ack/nak
    scl_low
    sda_out
    rts
.endproc

.proc i2c_read
    lda I2C_NUM_BITS     
    tax                 ; x = #bits
    sda_in

bitloop:
    lda SELECTPORT
    scl_high
    lda SELECTPORT
    and #I2C_SDA
    cmp #I2C_SDA
    beq high_bit
    asl I2C_BUFFER
    jmp clock_lo

high_bit:
    asl I2C_BUFFER
    lda #1
    ora I2C_BUFFER
    sta I2C_BUFFER

clock_lo:
    scl_low
    dex
    bne bitloop

ack_nak:
    ; Read ACK/NAK
    lda SELECTPORT
    scl_high
    ; read dataport for ack/nak
    scl_low
    sda_out
    rts
.endproc


; Read data from addr
; I2C_DEVICE, 
.proc i2c_byte_from_addr
    ; Set start condition
    i2c_start

; ---------------------------------------------
;
; Write out the device address with write flag
;
; ---------------------------------------------
    lda I2C_DEVICE
    asl
    sta I2C_BUFFER
    lda #I2C_FLAG_WRITE
    sta I2C_FLAGS
    lda #7
    sta I2C_NUM_BITS
    jsr i2c_write

; ---------------------------------------------
;
; Send the register address to read from
;
; ---------------------------------------------
    lda I2C_ADDR
    sta I2C_BUFFER
    lda #0
    sta I2C_FLAGS
    lda #8
    sta I2C_NUM_BITS
    jsr i2c_write
    i2c_stop

; ---------------------------------------------
;
; Write out the device address with read flag
;
; ---------------------------------------------
    i2c_start
    lda I2C_DEVICE
    asl
    sta I2C_BUFFER
    lda #I2C_FLAG_READ
    sta I2C_FLAGS
    lda #7
    sta I2C_NUM_BITS
    jsr i2c_write

; ---------------------------------------------
;
; Read the register data
;
; ---------------------------------------------
;    i2c_start
    lda #8
    sta I2C_NUM_BITS
    lda #0
    sta I2C_BUFFER_INDEX
    sta I2C_BUFFER_LENGTH
    jsr i2c_read
    i2c_stop
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
