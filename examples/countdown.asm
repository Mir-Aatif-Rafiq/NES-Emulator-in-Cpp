; Labels are resolved by the parser. BNE is encoded as a signed relative branch.
.ORG $8000

LDX #$05

loop:
DEX
BNE loop
STX $0200

.END
