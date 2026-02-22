.include "via6522_regs.inc"
.include "via6522_import.inc"
.include "lcd.inc"
.include "chaser.inc"

.import i2c_init, i2c_byte_from_addr
; .import I2C_DEVICE: zeropage, I2C_ADDR: zeropage
.import DATAIO, DATAPORT, SELECTPORT
.import var_init, var_push, var_pop

; .import via6522_init, via6522_timer_init, via6522_isr_ret

;.segment "ZEROPAGE"
; Key state
;KEYS: .byte 0

.segment "CODE"
.proc isr
    sei
;    lda $40
;    sta ORAIRA
;    sta ORBIRB
;    inc $40
    jsr via6522_isr_ret
    cli
    rti
.endproc

.proc main
    sei
;    jsr var_init
;    jsr via6522_init
;    jsr chaser_init
    jsr via6522_timer_init
;    jsr i2c_init
;    jsr lcd_init
:
;    jsr lcd_loop
;    jsr chaser_loop
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

.proc vttest
:
    lda #$11
    jsr var_push
    lda #$22
    jsr var_push
    lda #$33
    jsr var_push
    lda #$44
    jsr var_push
    lda #$55
    jsr var_push
    lda #$66
    jsr var_push
    lda #$77
    jsr var_push

    jsr var_pop
    jsr var_pop
    jsr var_pop
    jsr var_pop
    jsr var_pop
    jsr var_pop
    jsr var_pop
    jsr var_pop
    jmp :-
.endproc ; vttest

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
.segment "RESETVEC"
.word main
.segment "IRQVEC"
.word isr
