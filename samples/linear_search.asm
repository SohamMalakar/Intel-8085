
LDA F200H // the element to be searched
MOV B, A

LXI H, F100H // starting address of the array

LDA F300 // size of the array
MOV C, A

MVI D, 00H // flag

LOOP:
    MOV A, M
    CMP B
    JZ YES

    INX H
    DCR C
    JNZ LOOP

JMP END

YES:
    MVI D, 01H // found

END:
    RST 1
    HLT
