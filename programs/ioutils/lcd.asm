.include "i2c.inc"
.include "mcp23017.inc" 
.include "varstack.inc"
;.include "via6522.inc"

;DATAPORT = ORAIRA
;SELECTPORT = ORBIRB

LCD_RS    = %10000000       ; data=1 instruction=0
LCD_RW    = %01000000       ; r=1 w=0
LCD_E     = %00100000
NOT_LCD_E = %11011111

.segment "ZEROPAGE"
COUNT: .byte 0
LCD_INDEX: .byte 0
LCD_BASE: .word 0
LCD_DATA: .byte 0

.segment "CODE"
;.macro lcd_save
;    pha
;    ldy LCD_SAVE_INDEX
;    lda DATAPORT
;    sta LCD_SAVE, y
;    inc LCD_SAVE_INDEX
;    pla
;.endmacro

;.macro lcd_select
;    lda #SELECT_NONE
;    sta SELECTPORT
;    lda #$ff
;    sta DATAIO
;    lda #$00        ; set databus to input
;    sta DATAPORT
;    lda #SELECT_LCD_OUT ; select LCD
;    sta SELECTPORT
;.endmacro
;
;.macro lcd_deselect
;    lda #SELECT_NONE
;    sta SELECTPORT
;.endmacro

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
    ldx #0
    lda LCD_DATA
    jsr i2c_message
    rts
.endproc

; A = byte to write
.proc lcd_data
    ldx #LCD_RS
    lda LCD_DATA
    jsr i2c_message
    rts
.endproc

.proc lcd_counts
;    lcd_select
    lda #$00
    pha
loop:
    lda #$01        ; Clear
    sta LCD_DATA
    jsr lcd_instruction
    lda #$02        ; Home
    sta LCD_DATA
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
    cmp #$09        ; done with all the strings?
    bne loop
done:
    pla
;    lcd_deselect
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

.proc lcd_loop
    jsr lcd_counts
;    lda #$03
;    sta LCD_DATA
;    jsr lcd_instruction
    rts
.endproc

.proc lcd_init
;    lcd_select
    ; Set the MCP23017 port directions
    lda #$00         ; Set IODIRA to outputs
    jsr var_push
    lda #IODIRA    ; Reg address
    jsr var_push
    lda #$20        ; I2C Device
    jsr var_push
    jsr i2c_byte_to_addr

    lda #$00         ; Set IODIRB to outputs
    jsr var_push
    lda #IODIRB    ; Reg address
    jsr var_push
    lda #$20        ; I2C Device
    jsr var_push
    jsr i2c_byte_to_addr

    ; Force a good known reset
    lda #$03
    sta LCD_DATA
    jsr lcd_instruction
    jsr lcd_instruction
    jsr lcd_instruction

    ; Set a 4 bit interface
;    lda #$02         
;    sta DATAPORT
;    ora #LCD_E       ; clock it in
;    sta DATAPORT
;    eor #LCD_E       ; clock it in
;    sta DATAPORT
;    fixed_wait

    lda #$38         ; function set, DL=8 bits, N=2 lines
    sta LCD_DATA
    jsr lcd_instruction
    lda #$0e        ; display on
    sta LCD_DATA
    jsr lcd_instruction
    lda #$06        ; Entry mode
    sta LCD_DATA
    jsr lcd_instruction
    lda #$80        ; DDRAM Address
    sta LCD_DATA
    jsr lcd_instruction
    lda #$01        ; Clear
    sta LCD_DATA
    jsr lcd_instruction
    lda #$02        ; Home
    sta LCD_DATA
    jsr lcd_instruction
;    lcd_deselect
    rts
.endproc

; a = byte to send
; x = flags
.proc i2c_message
    ; Send the data byte
    phx             ; save the flags
    jsr var_push    ; queue the data
    lda #GPIOA     ; Select the data port
    jsr var_push
    lda #I2C_ADDR_LCD
    jsr var_push
    jsr i2c_byte_to_addr

    ; Send R/W and RS flags
    plx             ; get the flags
    txa
    and #NOT_LCD_E ; clear any accidental E
    phx
    jsr var_push    ; queue the flags
    lda #GPIOB     ; Select the flags port
    jsr var_push
    lda #I2C_ADDR_LCD
    jsr var_push
    jsr i2c_byte_to_addr

    ; Toggle the E line
    plx
    txa
    phx
    ora #LCD_E     ; Add E to flags
    jsr var_push    ; queue the flags
    lda #GPIOB     ; select the flags port
    jsr var_push
    lda #I2C_ADDR_LCD
    jsr var_push
    jsr i2c_byte_to_addr
    plx
    txa

    jsr var_push    ; queue the flags (no E)
    lda #GPIOB     ; select the flags port
    jsr var_push
    lda #I2C_ADDR_LCD
    jsr var_push
    jsr i2c_byte_to_addr
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

.export lcd_init, lcd_instruction, lcd_data, lcd_write_string, lcd_loop
