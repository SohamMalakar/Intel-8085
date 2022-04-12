MVI C, 00 // adjust variable

// lower nibble

LDA f100
CALL RSHIFT
MOV B, A

LDA f200
CALL RSHIFT

ADD B

JNC J1 // if CY == 0 then jump to J1

MVI C, 06
JMP J2

J1: CPI A0 // if A < A0 then jump to J2
JC J2

MVI C, 06
J2:

// higher nibble

LDA f100
ANI f0
MOV B, A

LDA f200
ANI f0

ADD B

JNC J3 // if CY == 0 then jump to J3

MOV A, C
ORI 60
MOV C, A
JMP J4

J3: CPI A0 // if A < A0 then jump to J4
JC J4

MOV A, C
ORI 60
MOV C, A
J4:

LDA f100
MOV B, A

LDA f200

ADD B
ADD C // adjust the value

STA f300

HLT

RSHIFT:
    ANI 0f
    RLC
    RLC
    RLC
    RLC
    RET
