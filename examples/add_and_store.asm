; A small end-to-end program for the text-file executor.
; .ORG tells the loader where the first instruction belongs in memory.
.ORG $8000

LDA #$05
CLC
ADC #$03
STA $0200

; .END belongs to the host-side executor. It is not a 6502 opcode.
.END
