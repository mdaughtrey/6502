.ifndef VIA6522_INC
.define VIA6522_INC
IOBASE = $4000
ORBIRB = IOBASE          ; Write = output register B, Read = input register B
ORAIRA = IOBASE + 1     ; Write = output register A, Read = input register A
DDRB = IOBASE + 2       ; direction register B
DDRA = IOBASE + 3       ; direction register A
T1CL = IOBASE + 4       ; Write = T1 Low order latches, Read = T1 Low order counter
T1CH = IOBASE + 5       ; T1 High order counter
T1LL = IOBASE + 6       ; T1 Low order latches
T1LH = IOBASE + 7       ; T1 High order latches
T2CL = IOBASE + 8       ; Write = T2 Low order latches, Read = T2 Low order counter
T2CH = IOBASE + 9       ; T2 High order counter
SR = IOBASE + 10        ; Shift register
ACR = IOBASE + 11       ; Auxiliary control register
PCR = IOBASE + 12       ; Peripheral control register
IFR = IOBASE + 13       ; Interrupt flag register
IER = IOBASE + 14       ; Interrupt enable register
ORAIRA0 = ORAIRA        ; ORAIRA no handshake


SELECT_NONE = %11111111
SELECT_KEYS = %11111110
; SELECT_I2C_SCK = %11111101
;SELECT_LCD_OUT = %11111011
SELECT_RESERVED0 = %10000000    ; I2C
SELECT_RESERVED1 = %01000000    ; I2C

; .export DATAIO, DATAPORT, I2C_ADDR, I2C_DEVICE, SELECTIO, SELECTPORT

.endif ; VIA6522_INC
