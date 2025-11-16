
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
; PARAMS:  .res 16
; Key state
KEYS: .byte 0
; MODE: .byte 0
COUNT: .byte 0
LCD_INDEX: .byte 0
LCD_BASE: .word 0
; LCD_COUNT: .byte 0

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
    jsr lcd_init
    jsr lcd_counts
;    lda #SELECT_LCD
;    sta SELECTPORT
;    lda #$ff
;    sta DATAIO

main_loop:
    jsr read_keys ; returns key bitmap in A
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
.proc lcd_instruction
    tax             ; save A
    lsr A           ; shift right 4
    lsr A
    lsr A
    lsr A
    ora #LCD_E        ; enable low
    sta DATAPORT    ; write D7-D4
    eor #LCD_E       ; enable hight
    sta DATAPORT    ; write D7-D4
    txa
    and #$0f        ; mask off lower bits
    ora #LCD_E
    sta DATAPORT    ; write D3-D0
    eor #LCD_E
    sta DATAPORT    ; write D3-D0
    rts
.endproc

; A = byte to write
.proc lcd_data
    tax             ; save A
    lsr A           ; shift right 4
    lsr A
    lsr A
    lsr A
    ora #LCD_RS   ; enable low
    sta DATAPORT    ; write D7-D4
    ora #LCD_E   ; enable low
    sta DATAPORT    ; write D7-D4
    eor #LCD_E       ; enable hight
    sta DATAPORT    ; write D7-D4
    txa
    and #$0f        ; mask off lower bits
    ora #LCD_RS   ; enable low
    sta DATAPORT    ; write D7-D4
    ora #LCD_E
    sta DATAPORT    ; write D3-D0
    eor #LCD_E
    sta DATAPORT    ; write D3-D0
    rts
.endproc

.proc lcd_counts
    lda #$00
    pha
;    sta PARAMS
loop:
    lda #$01        ; Clear
    jsr lcd_instruction
    lda #$02        ; Home
    jsr lcd_instruction
    pla
    tax
    pha
;    ldx PARAMS
    lda countslo, x
    sta LCD_BASE
    lda countshi, x
    sta LCD_BASE+1
    jsr lcd_write_string
    pla
    tax
    inx
    txa
    pha
;    inc PARAMS
    cmp #$0a
;    cmp PARAMS
    bne loop
done:
    rts
.endproc

; PARAM: string index
.proc lcd_write_string
    lda #$00
    sta LCD_INDEX
loop:
    lda LCD_INDEX
    tay
    lda (LCD_BASE), y  ; load the data
    beq done
    jsr lcd_data
    inc LCD_INDEX
    jmp loop
done:
    lda #$ff             ; Deselect LCD
    sta SELECTPORT       ; 
    rts
.endproc

.proc lcd_init
    lda #SELECT_LCD  ; select display
    sta SELECTPORT
    lda #$ff         ; set dataport to output
    sta DATAIO

    ; Force a good known reset
    lda #$03
    sta DATAPORT
    ora #LCD_E
    sta DATAPORT
    lda #$03
    sta DATAPORT
    ora #LCD_E
    sta DATAPORT
    lda #$03
    sta DATAPORT
    ora #LCD_E
    sta DATAPORT

    ; Set a 4 bit interface
    lda #$02         
    sta DATAPORT
    ora #LCD_E       ; clock it in
    sta DATAPORT
    eor #LCD_E       ; clock it in
    sta DATAPORT

    lda #$28         ; function set, DL=4 bits, N=2 lines
    jsr lcd_instruction
    lda #$0e        ; display on
    jsr lcd_instruction
    lda #$06        ; Entry mode
    jsr lcd_instruction
    lda #$80        ; DDRAM Address
    jsr lcd_instruction
;    lda #$01        ; Clear
;    jsr lcd_instruction
;    lda #$02        ; Home
;    jsr lcd_instruction
    rts
.endproc


.segment "RODATA"
one: .asciiz "one"
two: .asciiz "two"
three: .asciiz "three"
four: .asciiz "four"
five: .asciiz "five"
six: .asciiz "six"
seven: .asciiz "seven"
eight: .asciiz "eight"
nine: .asciiz "nine"
.define counts one, two, three, four, five, six, seven, eight, nine
countslo: .lobytes counts
countshi: .hibytes counts
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
