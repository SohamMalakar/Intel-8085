
LDA F100H // size of the array (input)
MOV B, A

LXI D, F300H // starting address of the array (input)
LXI H, F200H // store sqrt values here (output)

LOOP:
    LDAX D
    CALL SQRT
    MOV M, A
    
    INX H
    INX D
    DCR B
    JNZ LOOP

HLT

SQRT:
    MVI C, 01H

    LOOP1:
        SUB C
        JZ END

        INR C
        INR C

        JMP LOOP1
    
    END: MOV A, C
    RRC
    ANI 0FH
    ADI 01H
    RET
