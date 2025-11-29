.include "via6522.inc"
.include "i2c.inc"

.import DATAPORT, DATAIO, SELECTPORT, SELECTIO
.import var_push, var_pop

.segment "ZEROPAGE"
I2C_DEVICE:     .byte 0     ; I2C Device Address (7 bits, unshifted)
I2C_ADDR:       .byte 0     ; I2C Register Address
I2C_FLAGS:      .byte 0     ; Control flags
I2C_FLAG_READ   = %00000001
I2C_FLAG_WRITE  = %00000010
I2C_FLAG_NAK    = %10000000 ; Unexpected NAK from device
I2C_NUM_BITS:    .byte 0     ; number of bits to transact
I2C_BUFFER_INDEX: .byte 0   ; read/write buffer index
I2C_BUFFER_LENGTH: .byte 0  ; number of bytes processed
I2C_HEADER0:       .byte 0 ; Store device address
I2C_HEADER1:       .byte 0 ; Store register address
I2C_BUFFER: .res 8          ; read/write/buffer

.segment "CODE"
.proc   i2c_init
    lda SELECTPORT
    sda_high
    scl_low
    sta SELECTPORT
    rts
.endproc

.proc i2c_read_byte
    rts
.endproc

.proc i2c_bytes_to_addr
    rts
.endproc


; ---------------------------------------------
; Transact a frame
; a = data to send
; I2C_NUM_BITS = data length in bits
; I2C_FLAGS = read/write
; ---------------------------------------------
.proc i2c_write
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
    lda SELECTPORT
    and #I2C_SDA
    cmp #I2C_SDA
    bne ack                     ; low = ack
    lda #I2C_FLAG_NAK
    sta I2C_FLAGS
ack:
    scl_low
    sda_out
    rts
.endproc


; ---------------------------------------------
;
; Read an I2C frame and store into
; I2C_BUFFER[I2C_BUFFER_INDEX]
; Return:
;   I2C_BUFFER_INDEX
;   I2C_BUFFER_LENGTH
;
; ---------------------------------------------
.proc i2c_read
    lda I2C_NUM_BITS     
    tay                 ; y = #bits
    ldx I2C_BUFFER_INDEX    ; x = buffer index
    sda_in

bitloop:
    lda SELECTPORT      ; 
    scl_high            ; SCL High
    lda SELECTPORT      ; Read SDA
    and #I2C_SDA    
    cmp #I2C_SDA
    beq high_bit
    asl I2C_BUFFER, x   ; store 0 to buffer and shift
    jmp clock_lo

high_bit:
    ldx I2C_BUFFER_INDEX 
    asl I2C_BUFFER, x   ; store 1 to buffer and shift
    lda #1
    ora I2C_BUFFER, x
    sta I2C_BUFFER, x

clock_lo:
    scl_low             ; SCL Low
    dey
    bne bitloop         ; loop if bitcount (y) > 0

ack_nak:
    ; Read ACK/NAK
    lda SELECTPORT
    scl_high
    ; read dataport for ack/nak
    scl_low
    sda_out
    inc I2C_BUFFER_INDEX
    inc I2C_BUFFER_LENGTH
    rts
.endproc

; ---------------------------------------------
;
; Read data from I2C_DEVICE/I2C_ADDR
; VARSTACK
;   0: Register address
;   1: Device address
; Return:
;   I2C_BUFFER
;   I2C_BUFFER_LENGTH
;
; ---------------------------------------------
.proc i2c_byte_from_addr
    jsr var_pop             ; register
    sta I2C_HEADER1
    jsr var_pop             ; device
    asl
    sta I2C_HEADER0
    lda #0
    sta I2C_FLAGS
    ; Set start condition
    i2c_start

; ---------------------------------------------
;
; Write out the device address with write flag
;
; ---------------------------------------------
;    sta I2C_BUFFER
    lda #I2C_FLAG_WRITE
    sta I2C_FLAGS
    lda #7
    sta I2C_NUM_BITS
    ; Write out the I2C address
    lda I2C_HEADER0
    jsr i2c_write
    ; Test for error
;    lda I2C_FLAGS
;    cmp #I2C_FLAG_NAK
;    bne ack0
;    i2c_stop
;    rts                 ; got a NAK, bail

; ---------------------------------------------
;
; Set the register address 
;
; ---------------------------------------------
;ack0:
    lda #8
    sta I2C_NUM_BITS
;    lda #I2C_FLAG_READ
    lda #0
    sta I2C_FLAGS
    lda I2C_HEADER1
    jsr i2c_write
    ; Test for error
;    lda I2C_FLAGS
;    cmp #I2C_FLAG_NAK
;    bne ack1
;    i2c_stop
;    rts                 ; got a NAK, bail
;ack1:
    i2c_stop

; ---------------------------------------------
;
; Write out the device address with read flag
;
; ---------------------------------------------
    i2c_start
    lda #I2C_FLAG_READ
    sta I2C_FLAGS
    lda #7
    sta I2C_NUM_BITS
    lda I2C_HEADER0
    jsr i2c_write
    ; Test for error
;    lda I2C_FLAGS
;    cmp #I2C_FLAG_NAK
;    bne ack2
;    i2c_stop
;    rts                 ; got a NAK, bail

; ---------------------------------------------
;
; Read the register data
;
; ---------------------------------------------
;ack2:
    lda #8
    sta I2C_NUM_BITS
    lda #0
    sta I2C_BUFFER_INDEX
    sta I2C_BUFFER_LENGTH
    jsr i2c_read
    i2c_stop
    rts
.endproc


; ---------------------------------------------
;
; Write an I2C frame to I2C_DEVICE/I2C_ADD
; from I2C_BUFFER for I2C_LENGTH frames
; VARSTACK:
;   0: Device
;   1: Register Addr
;   2: Value
; Return:
;   I2C_BUFFER_INDEX
;   I2C_BUFFER_LENGTH
;
; ---------------------------------------------
.proc i2c_byte_to_addr
    ; Set start condition
    i2c_start

; ---------------------------------------------
;
; Write out the device address with write flag
;
; ---------------------------------------------
    lda #I2C_FLAG_WRITE
    sta I2C_FLAGS
    lda #7
    sta I2C_NUM_BITS
    jsr var_pop     ; Device Address
    jsr i2c_write

; ---------------------------------------------
;
; Set the register address 
;
; ---------------------------------------------
    lda #0
    sta I2C_FLAGS
    lda #8
    sta I2C_NUM_BITS
    jsr var_pop     ; Register Address
    jsr i2c_write

; ---------------------------------------------
;
; Set the register data 
;
; ---------------------------------------------
    lda #0
    sta I2C_FLAGS
    lda #8
    sta I2C_NUM_BITS
    jsr var_pop     ; Register Data
    jsr i2c_write
    i2c_stop

    rts
.endproc

.export i2c_byte_to_addr, i2c_byte_from_addr, i2c_init, I2C_ADDR, I2C_DEVICE
