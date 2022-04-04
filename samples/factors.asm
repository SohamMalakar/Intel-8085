LDA F100h
MVI H, 00h
MOV B, A
RRC
MOV C, A
LXI D, F200h

LOOP:
    MOV A, B
    CALL REM
    DCR C
    JNZ LOOP

HLT

REM:
    MOV L, C

    LOOP1:
        CMP L
        JC END
        SUB L
        JMP LOOP1

    END: CPI 00h
    JNZ NO
    MOV A, C
    STAX D

    INX D
    INR H

    MOV A, H
    STA F300h

    NO: RET
