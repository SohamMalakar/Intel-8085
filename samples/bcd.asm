// clear all the registers before doing anything.

lda f100
ani 0f
mov b, a

lda f200
ani 0f

add b
cpi 0a

jc j1
adi 06

j1: mov c, a

lda f100
ani f0
rrc
rrc
rrc
rrc
mov h, a

lda f200
ani f0
rrc
rrc
rrc
rrc
mov l, a

mov a, c
rlc
rlc
rlc
rlc

mov a, h
mov b, l
adc b

cpi 0a

jc j2
adi 06

j2: mov d, a

ani 0f
rlc
rlc
rlc
rlc

mov h, a

mov a, c
ani 0f

ora h

sta f301

mov a, d
ani f0
rlc
rlc
rlc
rlc

sta f300

// end of the program.
