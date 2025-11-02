
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

.segment "DATA"
; state variables
; STATE: .byte 0
; STATE_KEY = 0
; STATE_DISPLAY = 1 
; STATE_LOGIC = 2
DATAPORT = ORAIRA
DATAIO = DDRA
SELECTPORT = ORBIRB
SELECTIO = DDRB

SELECT_KEYS = %11111110
SELECT_DISPLAY = %11111101

; ---- VARIABLES ----
; Key state
KEYS: .byte 0
DISPLAY_LO: .byte 0
DISPLAY_HI: .byte 0

.segment "CODE"
.proc main
    lda #$00            ; reset state
;    sta STATE
    sta DISPLAY_LO      ; Clear sta DISPLAY_HI      ; Clear
    lda #$ff            ; Set select bus to output
    sta SELECTIO
    lda #$ff            ; Deselect all
    sta SELECTPORT       ; 

main_loop:
state_display:
;    lda #STATE_DISPLAY
;    cmp STATE
;    bne state_key
    jsr read_keys
    jsr logic
    jsr display
;    lda #STATE_KEY   ; set state to display
;    sta STATE
    jmp main_loop

;state_key:
;    lda #STATE_KEY
;    cmp STATE
;    bne state_logic
;    jsr read_keys
;    lda #STATE_LOGIC       ; set state to key
;    sta STATE
;    jmp main_loop

; state_logic:
;    jsr logic
;    lda #STATE_DISPLAY       ; set state to key
;    sta STATE
;    jmp main_loop
.endproc                ; main

.proc read_keys
    lda #$00            ; set dataport to input
    sta DATAIO
    lda #SELECT_KEYS     ; select keys
    sta SELECTPORT
    lda #$ff
    eor DATAPORT
    sta KEYS            ; store to memory
    lda #$0f
    and KEYS
    sta KEYS
    rts
.endproc

.proc display
    lda #$ff            ; Data port output
    sta DATAIO
    lda #SELECT_DISPLAY  ; select display
    sta SELECTPORT
    eor DISPLAY_LO      ; invert bits
    sta DATAPORT          ; write to leds
;    lda #$ff            ; load display 1
;    eor DISPLAY_HI      ; invert bits
;    sta ORAIRA          ; write to leds
    rts
.endproc

.proc logic
    lda KEYS
    tax
    lda led_maps, x
    sta DISPLAY_LO      ; Write to display memory
    rts
.endproc


.segment "RODATA"
led_maps:
.byte $00 ; 0000
.byte $03 ; 0001
.byte $0c ; 0010
.byte $3c ; 0011
.byte $30 ; 0100
.byte $33 ; 0101
.byte $3c ; 0110
.byte $3f ; 0111
.byte $c0 ; 1000
.byte $c3 ; 1001
.byte $cc ; 1010
.byte $cf ; 1011
.byte $c0 ; 1100
.byte $c3 ; 1101
.byte $fc ; 1110
.byte $ff ; 1111

.segment "RESETVEC"
.word $c000
