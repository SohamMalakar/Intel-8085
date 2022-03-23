
MVI H, 0AH
MVI L, 00H

LXI B, F100H

LOOP:
    LDAX B
    
    RAR
    JNC EVEN
    RAL
    
    ADD L
    MOV L, A
    
    EVEN: INX B
    DCR H
    JNZ LOOP

// RST 1
HLT
