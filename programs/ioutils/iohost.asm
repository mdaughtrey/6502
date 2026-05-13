.include "global_defs.inc"

.SEGMENT "IOHOST"
LIO_SIGNALS: .byte 1        ; Local I/O signals
HIO_SIGNALS: .byte 1        ; Host I/O signals
; STATUS_BITS for use in the SIGNAL regs
RX_READY = %10000000
TX_READY = %01000000
RX_DATA_MASK = %01111111
TX_DATA_MASK = %10111111

LIO_BUFFER: .byte 8     ; Local I/O buffer, for data to the host
HIO_BUFFER: .byte 8    ; Host I/O buffer, for data from the host
LIO_HEAD: .byte 1       ; Local I/O buffer head index
LIO_TAIL: .byte 1       ; Local I/O buffer tail index
HIO_HEAD: .byte 1       ; Host I/O buffer head index
HIO_TAIL: .byte 1       ; Host I/O buffer tail index



; initialize IOHOST
.proc iohost_init
    rts
.endproc
