LDA f100
MOV C, A
MVI B, 00
LXI D, 0000

LOOP:
    CALL XDAD
    DCR C
    JNZ LOOP

XCHG
SHLD f200

HLT

XDAD:
    MOV A, C
    ADD E

    MOV L, A
    MOV A, B

    ADC D
    MOV H, A
    MOV E, L
    MOV D, H
    RET
