
LDA f100

DCR A
MOV C, A // count

MVI D, 00
MVI E, 00

LOOP:
    RRC

    JC ODD
    JNC EVEN

    SKIP:

    DCR C
    MOV A, C
    JNZ LOOP

MOV A, D
STA f200

MOV A, E
STA f300

HLT

ODD:
    RLC
    ADD D
    MOV D, A
    JMP SKIP

EVEN:
    RLC
    ADD E
    MOV E, A
    JMP SKIP
