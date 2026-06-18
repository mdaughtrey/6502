.include "global_defs.inc"

.SEGMENT "IOHOST"
LIO_SIGNALS: .byte 0        ; Local I/O signals
; STATUS_BITS for use in the SIGNAL regs
TOHOST_READY = %10000000
; TX_READY = %01000000
; RX_DATA_MASK = %01111111
; TX_DATA_MASK = %10111111
LIO_TAIL: .byte 0       ; Local I/O buffer tail index
LIO_HEAD: .byte 0       ; Local I/O buffer head index
LIO_BUFFER: .res 8     ; Local I/O buffer, for data to the host


HIO_SIGNALS: .byte 0        ; Host I/O signals
FROMHOST_READY = %10000000
HIO_TAIL: .byte 0       ; Host I/O buffer tail index
HIO_HEAD: .byte 0       ; Host I/O buffer head index
HIO_BUFFER: .res 8    ; Host I/O buffer, for data from the host
BUFFER_MASK = %00000111 ; Mask for buffer indices (modulo 8)


.SEGMENT "CODE"

; initialize IOHOST
.proc iohost_init
    lda #0
    sta LIO_SIGNALS
    sta HIO_SIGNALS
    sta HIO_HEAD
    sta HIO_TAIL
    sta LIO_HEAD
    sta LIO_TAIL
    rts
.endproc

.proc iohost_isr
    rts
.endproc

.proc iohost_tx
    ; A: byte to send to host
    ; Returns:
    ;   C: 1 if byte was sent, 0 if buffer is full
    ;   LIO_BUFFER updated with byte if sent
    ldx LIO_HEAD
    sta LIO_BUFFER, x   ; Store byte in buffer at head index
    inx
    txa 
    and #BUFFER_MASK
    cmp LIO_TAIL        ; Check if head + 1 == tail (buffer full)
    beq @buffer_full
    sta LIO_HEAD        ; all good, store head and return
    lda #TOHOST_READY
    sta LIO_SIGNALS     ; Signal to host that data is ready
    sec
    rts
@buffer_full:    ; Buffer is full, clear TX_READY and return
    clc
    rts
.endproc

.proc iohost_rx_fill
    ; A: byte to send to host
    ; Returns:
    ;   C: 1 if byte was sent, 0 if buffer is full
    ;   LIO_BUFFER updated with byte if sent
    ldx HIO_HEAD
    sta HIO_BUFFER, x   ; Store byte in buffer at head index
    inx
    txa 
    and #BUFFER_MASK
    cmp HIO_TAIL        ; Check if head + 1 == tail (buffer full)
    beq @buffer_full
    sta HIO_HEAD        ; all good, store head and return
    lda #FROMHOST_READY
    sta HIO_SIGNALS     ; Signal to host that data is ready
    sec
    rts
@buffer_full:    ; Buffer is full, clear TX_READY and return
    clc
    rts
.endproc

.proc iohost_rx
    ; Returns:
    ;   A: byte received from host
    ;   C: 1 if byte was received, 0 if buffer is empty
    lda HIO_SIGNALS
    and #FROMHOST_READY
    beq @buffer_empty
    ldx HIO_TAIL
    ldy HIO_BUFFER, x   ; Load byte from buffer at tail index
    inx
    txa
    and #BUFFER_MASK
    sta HIO_TAIL        ; Store updated tail index
    lda HIO_SIGNALS
    and #~FROMHOST_READY & $ff
    sta HIO_SIGNALS     ; Clear FROMHOST_READY to indicate we've read the byte
    tya
    sec
    rts
@buffer_empty:
    clc
    rts
.endproc

; fake host
; returns data in A, C if buffer is empty
.proc iohost_tx_drain
    lda LIO_SIGNALS
    and #TOHOST_READY
    beq @buffer_empty
    and #~TOHOST_READY & $ff
    sta LIO_SIGNALS     ; Clear TOHOST_READY to indicate we've read the byte
    ldx LIO_TAIL
    ldy LIO_BUFFER, x
    inx
    txa
    and #BUFFER_MASK
    sta LIO_TAIL
    tya
    rts
@buffer_empty:
    sec
    rts

.endproc


.proc iohost_loop
;    jsr iohost_tx_drain
;    lda #'M'
;    jsr iohost_rx_fill
;
    jsr iohost_rx
    bcc @no_data
    jsr iohost_tx
@no_data:
    rts
.endproc

.export iohost_init, iohost_isr, iohost_tx, iohost_rx, iohost_loop
