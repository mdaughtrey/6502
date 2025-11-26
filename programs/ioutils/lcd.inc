LCD_D0    = %00000001
LCD_D1    = %00000010
LCD_D2    = %00000100
LCD_D3    = %00001000
LCD_RS    = %00010000
LCD_RW    = %00100000       ; read=1 write=0
LCD_E     = %01000000
LCD_BUSY  = %10000000

.segment "ZEROPAGE"
COUNT: .byte 0
LCD_INDEX: .byte 0
LCD_BASE: .word 0
LCD_DATA: .byte 0

.segment "CODE"
.macro lcd_save
    pha
    ldy LCD_SAVE_INDEX
    lda DATAPORT
    sta LCD_SAVE, y
    inc LCD_SAVE_INDEX
    pla
.endmacro

.macro lcd_select
    lda #SELECT_NONE
    sta SELECTPORT
    lda #$ff
    sta DATAIO
    lda #$00        ; set databus to input
    sta DATAPORT
    lda #SELECT_LCD_OUT ; select LCD
    sta SELECTPORT
.endmacro

.macro lcd_deselect
    lda #SELECT_NONE
    sta SELECTPORT
.endmacro

; Cycles: 12 + (8*count) 
; Count 10 = 92 cycles * 8khz = 11.5ms
.macro fixed_wait
    pha         ; 3
    lda #$0a    ; 2
    sta COUNT   ; 3
:
    dec COUNT   ; 5
    bne :-      ; 2 + 1 + 1
    pla         ; 4
.endmacro

.macro lcd_busywait0
    pha
;    lda #SELECT_LCD_IN
;    sta SELECTPORT
;    lda #%11110111  ; Bit 3 input to read DB7 from LCD
;    sta DATAIO
:
    lda DATAPORT    
    ora #LCD_RW      ; assert READ
    sta DATAPORT
    ora #LCD_E       ; assert E
    sta DATAPORT
    eor #LCD_E       ; deassert E
    sta DATAPORT
    lda DATAPORT    ; save the high bits
    tax             ; save to x
    lda DATAPORT    ; Clock in the low 4 bits, ignore
    ora #LCD_E
    sta DATAPORT
    eor #LCD_E
    sta DATAPORT
    eor #LCD_RW
    sta DATAPORT
    txa             ; get from x
    and #%10000000  ; DB7 from the first read is mapped to bit 7
    bne :-          ; not zero, loop again
;    lda #$ff         ; otherwise return DATAIO to output
;    sta DATAIO
;    lda #SELECT_LCD_OUT
;    sta SELECTPORT
    pla
    rts
.endmacro

.proc lcd_instruction
;    lcd_busywait
    fixed_wait
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
;   lcd_busywait
    fixed_wait
    lda LCD_DATA
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
    lcd_select
    lda #$00
    pha
loop:
    lda #$01        ; Clear
    jsr lcd_instruction
    lda #$02        ; Home
    jsr lcd_instruction
    pla             ; Pull A
    tax             ; Save to X (string index)
    pha             ; push a
    lda countslo, x     ; load pointer to string[x]
    sta LCD_BASE        ; ...
    lda countshi, x     ; ...
    sta LCD_BASE+1      ; ...
    jsr lcd_write_string     ; write out the string
    pla             ; increment the string index
    tax
    inx
    txa
    pha
    cmp #$0a        ; done with all the strings?
    bne loop
done:
    lcd_deselect
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
    sta LCD_DATA
    jsr lcd_data
    inc LCD_INDEX
    jmp loop
done:
;    lda #$ff             ; Deselect LCD
;    sta SELECTPORT       ; 
    rts
.endproc

.proc lcd_init
    lcd_select
    ; Force a good known reset
    ldx #$03
    ldy #($03 | LCD_E)
    txa
    sta DATAPORT    ; Reset 1
    tya             ; E High
    sta DATAPORT
    txa             ; E low
    sta DATAPORT
    fixed_wait
    tya             ; E high
    sta DATAPORT
    txa             ; E low
    sta DATAPORT
    fixed_wait
    tya             ; E high
    sta DATAPORT        
    txa             ; E low
    sta DATAPORT
    fixed_wait

    ; Set a 4 bit interface
    lda #$02         
    sta DATAPORT
    ora #LCD_E       ; clock it in
    sta DATAPORT
    eor #LCD_E       ; clock it in
    sta DATAPORT
    fixed_wait

    lda #$28         ; function set, DL=4 bits, N=2 lines
    jsr lcd_instruction
    lda #$0e        ; display on
    jsr lcd_instruction
    lda #$06        ; Entry mode
    jsr lcd_instruction
    lda #$80        ; DDRAM Address
    jsr lcd_instruction
    lda #$01        ; Clear
    jsr lcd_instruction
    lda #$02        ; Home
    jsr lcd_instruction
    lcd_deselect
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
