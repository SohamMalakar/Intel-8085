
MVI A, 00
STA f200

LDA f100 // input
MOV L, A
RRC
ANI 7f

MOV D, A

MVI E, 00 // flag
MVI B, 01 // iterator
MVI C, 00 // counter

LOOP:
    CMP B
    JC OUT // if A < B, jump out
    MOV A, L
    CALL REM
    MOV A, D
    INR B
    JMP LOOP

OUT: MOV A, C
CPI 01
JNZ NOT
INR E
MOV A, E
STA f200
NOT: HLT

REM:
    MOV H, B

    LOOP1:
        CMP H
        JC END // if A < H, jump out
        SUB H
        JMP LOOP1

    END: CPI 00
    JNZ NO
    INR C
    NO: RET
