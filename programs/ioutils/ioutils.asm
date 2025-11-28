
.include    "io.inc"
;.import     "i2c.inc", i2c_init
.segment "CODE"
    jmp     main
.include    "i2c.inc"
; .include    "lcd.inc"

.segment "ZEROPAGE"
; PARAMS:  .res 16
; Key state
KEYS: .byte 0

.segment "CODE"
.proc main
    lda #$ff            ; Set select bus to output
    sta SELECTIO
    lda #$ff            ; Deselect all
    sta SELECTPORT       ; 
    jsr i2c_init
;    jsr chaser_init
;    jsr lcd_init
    lda #$20
    sta I2C_DEVICE
    lda #$00
    sta I2C_ADDR
    jsr i2c_byte_from_addr
;    inc I2C_DATA0
:
    jmp :-
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

; .proc display
;     tax
;     lda #$ff            ; Data port output
;     sta DATAIO
;     lda #SELECT_DISPLAY  ; select display
;     sta SELECTPORT
;     txa
;     eor #$ff
;     sta DATAPORT          ; write to leds
;     rts
; .endproc

; Call A: key bitmap
; Return: A: display bitmap
;.proc logic
;    tax
;    lda led_maps, x
;    rts
;.endproc

; A = byte to write


; j.proc chaser_init
; j    lda #SELECT_DISPLAY
; j    sta SELECTPORT
; j    lda #$ff
; j    sta DATAIO
; j    rts
; j.endproc
; j
; j.proc chaser
; j    fixed_wait
; j    inc DATAPORT
; j    rts
; j.endproc

.segment "RESETVEC"
.word $c200
