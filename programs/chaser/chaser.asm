.segment "CODE"

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

.proc main
    ldx #$00
    stx $00
    inx
    stx $01
    lda #$ff
    sta DDRB
    sta DDRA
loop:
    lda #$01
    eor $01
    sta ORBIRB
    lda #$01
    eor $00
    sta ORAIRA
    asl $01
    bcc reg2
    lda #$01
    sta $00
    jmp loop
reg2:
    asl $00
    bcc loop
    lda #$01
    sta $01
    jmp loop

.endproc
