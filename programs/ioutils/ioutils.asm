
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

.segment "ZEROPAGE"
PARAMS:  .res 16
; Key state
KEYS: .byte 0
; MODE: .byte 0
COUNT: .byte 0

.segment "DATA"
DATAPORT = ORAIRA
DATAIO = DDRA
SELECTPORT = ORBIRB
SELECTIO = DDRB

SELECT_KEYS = %11111110
SELECT_DISPLAY = %11111101
SELECT_LCD = %11111011

LCD_D0    = %00000001
LCD_D1    = %00000010
LCD_D2    = %00000100
LCD_D3    = %00001000
LCD_RS    = %00010000
LCD_RW    = %00100000
LCD_E     = %01000000
LCD_BUSY  = %10000000

.segment "CODE"
.proc main
    lda #$ff            ; Set select bus to output
    sta SELECTIO
    lda #$ff            ; Deselect all
    sta SELECTPORT       ; 
;    jsr lcd_init
;    lda #SELECT_LCD
;    sta SELECTPORT
;    lda #$ff
;    sta DATAIO

main_loop:
    jsr read_keys ; returns key bitmap in A
    sta KEYS
    lda #SELECT_LCD
    sta SELECTPORT 
    lda #$ff
    sta DATAIO
    lda KEYS
    cmp #$01
    bne ml1
    lda #LCD_RS
    sta DATAPORT
ml1:
    cmp #$02
    bne ml2
    lda #LCD_RW
    sta DATAPORT
ml2:
    cmp #$04
    bne ml3
    lda #LCD_E
    sta DATAPORT
ml3:
    

;    jsr logic ; returns A: display bitmap
;    jsr display
    jmp main_loop

.endproc                ; main

; return: a = keys pressed bitmask (....bbbb)
.proc read_keys
    lda #$00            ; set dataport to input
    sta DATAIO
    lda #SELECT_KEYS     ; select keys
    sta SELECTPORT
    lda DATAPORT
    eor #$ff
    and #$0f            ; mask off the bits we don't need
    rts
.endproc

.proc display
    tax
    lda #$ff            ; Data port output
    sta DATAIO
    lda #SELECT_DISPLAY  ; select display
    sta SELECTPORT
    txa
    eor #$ff
    sta DATAPORT          ; write to leds
    rts
.endproc

; Call A: key bitmap
; Return: A: display bitmap
;.proc logic
;    tax
;    lda led_maps, x
;    rts
;.endproc

; A = byte to write
.proc lcd_write_byte
    tax             ; save A
    lsr A           ; shift right 4
    lsr A
    lsr A
    lsr A
    ora LCD_E        ; enable low
    sta DATAPORT    ; write D7-D4
    eor LCD_E       ; enable hight
    txa
    and #$0f        ; mask off lower bits
    ora LCD_E
    sta DATAPORT    ; write D3-D0
    eor LCD_E
    rts
.endproc

.proc lcd_init
    lda #SELECT_LCD  ; select display
    sta SELECTPORT
    lda #$ff         ; set dataport to output
    sta DATAIO
    lda #$28         ; 4-bit interface, 2 lines
    jsr lcd_write_byte
    lda #$0f         ; display on
    jsr lcd_write_byte
    rts
.endproc


; .segment "RODATA"
; led_maps:
; .byte $00 ; 0000
; .byte $03 ; 0001
; .byte $0c ; 0010
; .byte $3c ; 0011
; .byte $30 ; 0100
; .byte $33 ; 0101
; .byte $3c ; 0110
; .byte $3f ; 0111
; .byte $c0 ; 1000
; .byte $c3 ; 1001
; .byte $cc ; 1010
; .byte $cf ; 1011
; .byte $c0 ; 1100
; .byte $c3 ; 1101
; .byte $fc ; 1110
; .byte $ff ; 1111

.segment "RESETVEC"
.word $c200
