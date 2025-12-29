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
;    lda #(ACR_T1B | ACR_L1 | ACR_L0)      ; PB Continuous Interrupts
;    lda #0
;    sta T1CL
;    sta T1CH
    lda #ACR_T1B
    sta ACR             ; Store to Aux Control Register
    lda #(IER_TIMER1 | IER_SET)  ; Enable interrupt
    sta IER             ; ...
    cli
    lda #<$00ff            ; Set the latches
    sta T1CL            ; ...
    lda #>$00ff
    sta T1CH            ; ...
    rts
.endproc

.proc via6522_isr_ret
;    lda IFR             ; read to clear the interrupt
    lda T1CL            ; read to clear the interrupt
    rts
.endproc

.export via6522_init, via6522_timer_init, via6522_isr_ret
.export DATAPORT, DATAIO, SELECTPORT, SELECTIO
