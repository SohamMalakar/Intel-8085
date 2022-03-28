// Divide two numbers

LDA F100H // Divisor
MOV B, A

LDA F200H // Dividend

MVI C, 00H // Clear quotient

LOOP:
    CMP B // Compare result
    JC END

    SUB B // Subtract divisor from dividend
    INR C // Increment quotient
    JMP LOOP

END: HLT
