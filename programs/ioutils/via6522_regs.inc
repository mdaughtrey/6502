.ifndef VIA6522_INC
.define VIA6522_INC
IOBASE = $5000
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

; Interrupt Enable Register
IER_SET_CLEAR   = %1000000
IER_TIMER1      = %01000000     ; Timer1
IER_TIMER2      = %00100000     ; Timer2
IER_CB1         = %00010000     ; Peripheral B Control 1 (read handshake)
IER_CB2         = %00001000     ; Peripheral B Control 2 (write handshake)
IER_SHIFT_REG   = %00000100     ; Shift register
IER_CA1         = %00000010     ; Peripheral A Control 1 (read handshake)
IER_CA2         = %00000001     ; Peripheral A Control 2 (write handshake)

; Auxiliary Control Register
ACR_T1A         = %10000000     ; Timer1 Control A
ACR_T1B         = %01000000     ; Timer1 Control B
ACR_T2          = %00100000     ; Timer2 Control 
ACR_SRA         = %00010000     ; Shift Register Control A
ACR_SRB         = %00001000     ; Shift Register Control B
ACR_SRC         = %00000100     ; Shift Register Control C
ACR_PB          = %00000010     ; Peripheral B Latch Enable
ACR_PA          = %00000001     ; Peripheral A Latch Enable

SELECT_NONE = %11111111
SELECT_KEYS = %11111110
SELECT_RESERVED0 = %10000000    ; I2C
SELECT_RESERVED1 = %01000000    ; I2C


.endif ; VIA6522_INC
