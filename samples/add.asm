MVI A, FEH
MVI B, 27H

ADD B
STA F100H

MVI A, 00H

JNC NC

MVI A, 01H
NC: STA F101H

RST 1
HLT