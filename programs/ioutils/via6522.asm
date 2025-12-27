.include "via6522_regs.inc"

.segment "DATA"
DATAPORT = ORAIRA
DATAIO = DDRA
SELECTPORT = ORBIRB
SELECTIO = DDRB

.segment "CODE"
.proc via6522_init
    lda #$ff            ; Set select bus to output
    sta SELECTIO
    lda #$ff            ; Deselect all
    sta SELECTPORT       ; 
    rts
.endproc

;
; Set up a 10ms timer, kicks off an interrupt
.proc via6522_timer_init
    lda #(ACR_T1B)      ; PB Continuous Interrupts
    sta ACR             ; Store to Aux Control Register
    lda #$01            ; Set the latches
    sta T1CL            ; ...
    lda #$00            ; Set the latches
    sta T1CH            ; ...
    lda #IER_TIMER1     ; Enable interrupt
    sta IER             ; ...
    rts
.endproc

.proc via6522_isr_ret
    lda T1CL            ; read to clear the interrupt
    rts
.endproc

.export via6522_init, via6522_timer_init, via6522_isr_ret
.export DATAPORT, DATAIO, SELECTPORT, SELECTIO
