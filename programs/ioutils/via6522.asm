.include "via6522.inc"

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

.export via6522_init
.export DATAPORT, DATAIO, SELECTPORT, SELECTIO
