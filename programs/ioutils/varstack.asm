.segment "ZEROPAGE"
VARSTACK_MARKER: .byte $bb
VARSTACK: .res 8
STACK_INDEX: .byte 0

.segment "CODE"
; a = value to push

.proc var_init
    lda #0
    sta STACK_INDEX
    rts
.endproc

.proc var_push
    ldx STACK_INDEX
    sta VARSTACK, x
    inc STACK_INDEX
    rts
.endproc

; pop to a
.proc var_pop
    lda #0
    cmp STACK_INDEX
    beq nodec
    dec STACK_INDEX
nodec:
    ldx STACK_INDEX
    lda VARSTACK, x
    rts
.endproc

.export var_init, var_push, var_pop
