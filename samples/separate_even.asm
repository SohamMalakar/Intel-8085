
MVI H, 0AH

LXI B, F100H
LXI D, F200H

LOOP:
    LDAX B

    RAR
    JC ODD
    RAL

    STAX D
    INX D

    ODD: INX B
    DCR H
    JNZ LOOP

// RST 1
HLT
