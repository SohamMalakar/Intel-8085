
LDA f100

MOV B, A
MOV C, A

MVI D, 04 // counter
MVI E, 00 // flag

LOOP:
    MOV A, C
    RLC
    MOV C, A

    XRA B
    ANI 01
    JNZ NO

    MOV A, B
    RRC
    MOV B, A

    DCR D
    JNZ LOOP

INR E

NO: MOV A, E
STA f200

HLT
