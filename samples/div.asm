// Divide two numbers

MVI A, 00H
MVI D, 00H

// Two numbers
MVI B, 25H
MVI C, 05H

JMP START

CON:
    JNZ OUT
    JZ IN

START:
    ADD C
    CMP B
    JNC CON
IN: INR D
    JMP START

OUT: HLT
