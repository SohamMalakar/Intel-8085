// Multiply two numbers

MVI A, 00H

// Two numbers
MVI B, FFH
MVI C, FFH

LOOP:
    ADD B
    JNC NC
    INR D
NC:
    DCR C // Count down
    JNZ LOOP // Loop if not zero
HLT
