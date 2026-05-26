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
HIO_SIGNALS: .byte 0        ; Host I/O signals
HIO_TAIL: .byte 0       ; Host I/O buffer tail index
HIO_HEAD: .byte 0       ; Host I/O buffer head index

LIO_BUFFER: .res 8     ; Local I/O buffer, for data to the host
HIO_BUFFER: .res 8    ; Host I/O buffer, for data from the host
BUFFER_MASK = %00000111 ; Mask for buffer indices (modulo 8)


.SEGMENT "CODE"

; initialize IOHOST
.proc iohost_init
    lda #0
    sta LIO_SIGNALS
    sta HIO_SIGNALS
    sta LIO_HEAD
    sta LIO_TAIL
    sta HIO_HEAD
    sta HIO_TAIL
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
    stx LIO_HEAD        ; all good, store head and return
    lda #TOHOST_READY
    sta LIO_SIGNALS     ; Signal to host that data is ready
    sec
    rts
@buffer_full:    ; Buffer is full, clear TX_READY and return
    clc
    rts
.endproc

.proc iohost_rx    ; Returns:
    ;   A: byte received from host, or 0 if buffer is empty
    ;   C: 1 if byte was received, 0 if buffer is empty
    ;   HIO_SIGNALS updated with RX_READY cleared if buffer is empty
    rts
.endproc

.proc iohost_loop
    lda #'M'
    jsr iohost_tx
    rts
.endproc

.export iohost_init, iohost_isr, iohost_tx, iohost_rx, iohost_loop
